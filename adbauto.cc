#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>

#include <iostream>
#include <array>

#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <range/v3/all.hpp>

//TODO:
// 1. 单例守护进程
// 2. 写syslog
// 3. 支持设置连接的adb server host和port(localhost:5037)

#define RUN_IN_DAEMON 1

#if RUN_IN_DAEMON
bool g_daemon_process = false;
#endif
bool g_verbose = false;
int g_sleep_time = 1;

void die(int exit_code, std::string msg) {
  std::cerr << msg << std::endl;
  exit(exit_code);
}
void usage() {
  std::cout << "adbauto [options] command [args...]\n";
  std::cout << "\n";
  std::cout << "options:\n";
  std::cout << " -h:\n";
  std::cout << " --help:    显示本帮助文档\n";
  std::cout << "\n";
#if RUN_IN_DAEMON
  std::cout << " -d:\n";
  std::cout << " --daemon:  守护进程运行本程序adbauto\n";
  std::cout << "\n";
#endif
  std::cout << " -v:\n";
  std::cout << " -verbose:  显示一些冗余信息, 例如: 打印触发的命令\n";
  std::cout << "\n";
  std::cout << " -V:\n";
  std::cout << " --version: 显示版本信息\n";
  std::cout << "\n";
  std::cout << "examples:\n";
  std::cout << " adbauto notify-send 'android device online: {}'\n";
  std::cout << " adbauto adb -s {} shell setprop name value\n";
  std::cout << " adbauto adb -s {} uninstall com.UCMobile.mytest\n";
  std::cout << " adbauto scrcpy -s {}\n";
  std::cout << " adbauto /path/to/do_somthing_script.sh {}\n";
  std::cout << "\n";
  std::cout << " >>> do_somthing_script.sh<<<\n";
  std::cout << " 1 #!/usr/bin/env bash\n";
  std::cout << " 2 android_device=\"$1\"\n";
  std::cout << " 3 adb -s $android_device shell rm /data/local/tmp/mylog.txt\n";
  std::cout << " 4 adb -s $android_device shell setprop delay 250\n";
  std::cout << " 5 notify-send \"android device online: $android_device\" # 桌面通知Android连上\n";
  std::cout << " 6 scrcpy -s $android_device # 桌面投屏显示Android手机截面\n";
  std::cout << "\n";
  std::cout << "\n";
  std::cout << std::endl;
}

void print_version() {
  std::cout << "0.0.1" << std::endl;
}

void print_vector_string(std::ostream& out, std::vector<std::string> const& vstrs) {
  out << "[";
  for (auto const& str: vstrs) {
    out << str << ", ";
  }
  out << "]" << std::endl;
}

bool isnewline(char x) { return x == '\n' || x == '\r'; }

void execute_process_for_device(std::vector<char> data, std::vector<std::string> const& commands) {
  std::string_view devices_info(data.data(), data.size());
  auto devices_view = devices_info
      | ranges::views::split_when(&isnewline)
      | ranges::views::transform([](auto&& rg){
          return std::string_view(&*rg.begin(), ranges::distance(rg))
            | ranges::views::split_when((int(*)(int))&std::isspace)
            | ranges::views::transform([](auto&& rg) {
                return std::string_view(&*rg.begin(), ranges::distance(rg));
              }) ;
        });
  for (auto const& line: devices_view) {
    auto col = line.begin();
    if (col != line.end()) {
      std::string_view serial(*col);
      if (++col != line.end()) {
        std::string_view status(*col);
        if (status == "device") {
          if (commands.size() == 0) {
            std::cout << serial << " is online!" << std::endl;
          } else {
            std::vector<std::string> replace_arguments;
            for (auto i = std::next(commands.begin(), 1); i != commands.end(); ++i) {
              replace_arguments.push_back(boost::algorithm::replace_all_copy(*i, "{}", serial));
            }
            std::string command = boost::algorithm::replace_all_copy(commands[0], "{}", serial);

            boost::filesystem::path cmd("");
            if (command.find('/') != std::string::npos) {
              cmd = boost::filesystem::path(command);
              if (g_verbose) { std::cout << cmd << " "; print_vector_string(std::cout, replace_arguments); }
              boost::process::child child(cmd, boost::process::args=replace_arguments);
              child.detach();
            } else {
              cmd = boost::process::search_path(command);
              if (!exists(cmd) || command.find('=') != std::string::npos) {
                std::string shell_str = command;
                for (auto const& i: replace_arguments) {
                  shell_str = shell_str + " " + i;
                }
                if (g_verbose) { std::cout << "bash -c '" << shell_str << "'" << std::endl; }
                boost::process::child child(boost::process::search_path("bash"), "-c", boost::process::args+=shell_str);
                child.detach();
              } else {
                if (g_verbose) { std::cout << cmd << " "; print_vector_string(std::cout, replace_arguments); }
                boost::process::child child(cmd, boost::process::args=replace_arguments);
                child.detach();
              }
            }
          }
        }
      }
    }
  }
}

void TrackAndroidDevice(std::string const& host, int port, std::vector<std::string> const& commands) {
  try {
    using boost::asio::ip::tcp;
    boost::asio::io_service io;
    tcp::socket socket(io);
    tcp::endpoint endpoint(boost::asio::ip::address::from_string(host), port);
    boost::system::error_code error;
    socket.connect(endpoint);

    // write
    std::string_view request_msg("0012host:track-devices");
    boost::system::error_code ignored_error;
    boost::asio::write(socket, boost::asio::buffer(request_msg), ignored_error);

    // read
    std::array<char, 128> buf;
    std::vector<char> response_data;
    std::size_t read_len = boost::asio::read(socket, boost::asio::buffer(buf), boost::asio::transfer_at_least(4), error);
    if (error) {
      return;
    }

    if (std::string_view(buf.data(), 4) != "OKAY" ) {
      std::cerr << "adbd response_data:" << std::string_view(buf.data(), buf.size()) << std::endl;
      return;
    }

    g_sleep_time = 1;

    if (read_len > 4) {
      response_data.insert(response_data.end(), buf.data() + 4, buf.data() + read_len);
    }
    while(true) {
      while (response_data.size() > 4) {
        int data_len = std::stoi(std::string(response_data.data(), 4), 0, 16);
        response_data.erase(response_data.begin(), std::next(response_data.begin(), 4));
        if (data_len != 0 && response_data.size() >= data_len) {
          std::vector<char> data;
          data.insert(data.end(), response_data.begin(), std::next(response_data.begin(), data_len));
          response_data.erase(response_data.begin(), std::next(response_data.begin(), data_len));

          execute_process_for_device(std::move(data), commands);
        }
      }

      read_len = socket.read_some(boost::asio::buffer(buf), error);
      if (read_len < 0 || error) { break; }
      if (read_len > 0) {
        response_data.insert(response_data.end(), buf.data(), buf.data() + read_len);
      }
    }
  } catch (std::exception& e) {
    std::cerr << host << ":" << port << " " << e.what() << std::endl;
  }
}

int main(int argc, char* const argv[]) {

  int command_index = 1;
  for (; command_index < argc; ++command_index) {
    std::string_view arg(argv[command_index]);
    if (arg == "-h" || arg == "--help") {
      usage();
      exit(0);
#if RUN_IN_DAEMON
    } else if (arg == "-d" || arg == "--daemon") {
      g_daemon_process = true;
    } else if (arg == "--no-daemon") {
      g_daemon_process = false;
#endif
    } else if (arg == "-v" || arg == "--verbose") {
      g_verbose = true;
    } else if (arg == "-V" || arg == "--version") {
      print_version();
      exit(0);
    } else if (arg == "--") {
      ++command_index;
      break;
    } else if (arg[0] == '-') {
      die(1, std::string("unknown option: ") + arg.data());
    } else {
      break;
    }
  }

#if RUN_IN_DAEMON
  pid_t pid;
  struct sigaction sa;
  struct rlimit rl;
  int fd0, fd1, fd2;


  if (g_daemon_process) {
    umask(0);

    if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
      die(1, "can't get file limit.");
    }

    if ((pid = fork()) < 0) {
      die(1, "can't fork");
    } else if (pid != 0) {
      exit(0);
    }

    setsid();

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGHUP, &sa, NULL) < 0) {
      die(1, "can't ignore SIGHUP");
    }

    if ((pid = fork()) < 0) {
      die(1, "can't fork");
    } else if (pid != 0) {
      exit(0);
    }

    // chdir("/");

    if (rl.rlim_max == RLIM_INFINITY) {
      rl.rlim_max = 1024;
    }
    for (int i = 0; i < rl.rlim_max; ++i) {
      close(i);
    }

    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(0);
    fd2 = dup(0);
  }
#endif

  std::vector<std::string> commands;
  if (command_index < argc) {
    commands.insert(commands.end(), argv + command_index, argv + argc);
  }
  while (true) {
    TrackAndroidDevice("127.0.0.1", 5037, commands);
    if (g_sleep_time < 60) {
      g_sleep_time = g_sleep_time + 1;
    } else {
      g_sleep_time = 60;
    }
    sleep(g_sleep_time);
  }
  return 1;
}
