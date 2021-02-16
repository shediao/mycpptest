

#include <boost/process.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/phoenix/phoenix.hpp>
#include <boost/utility/string_ref.hpp>
#include <iostream>
#include <iterator>

namespace bp = boost::process;
using boost::phoenix::arg_names::arg1;
using boost::phoenix::arg_names::arg2;
using boost::phoenix::arg_names::arg3;
using boost::phoenix::arg_names::arg4;
using boost::phoenix::arg_names::arg5;

class ExecResult{
public:
  ExecResult(int exit_code, std::string stdout_raw, std::string stderr_raw)
    :exit_code(exit_code),
     stdout_raw(std::move(stdout_raw)),
     stderr_raw(std::move(stderr_raw)) {

    auto begin = find_if(this->stdout_raw.begin(), this->stdout_raw.end(), [](auto x) { return !std::isspace(x); });
    auto end = find_if(this->stdout_raw.rbegin(), this->stdout_raw.rend(), [](auto x) { return !std::isspace(x); }).base();
    this->stdout_str = boost::string_ref(&*begin, std::distance(begin, end));
    auto b = this->stdout_str.begin();
    for(auto i = this->stdout_str.begin(); i != this->stdout_str.end(); ++i) {
      if (*i == '\n') {
        auto e = i;
        if (*(e-1) == '\r') { --e; }
        boost::string_ref device(&*b, std::distance(b, e));
        if (device.size() > 0) {
          lines.push_back(device);
        }
        b = i; ++b;
      }
    }
    if (b != this->stdout_str.end()) {
      lines.push_back(boost::string_ref(&*b, std::distance(b, this->stdout_str.end())));
    }
  }
  ExecResult() = delete;
  ExecResult(const ExecResult&) = delete;
  ExecResult(ExecResult&&) = default;

  int exit_code = 0;
  std::string stdout_raw;
  std::string stderr_raw;

  boost::string_ref stdout_str;
  std::vector<boost::string_ref> lines;
};

std::ostream& operator<<(std::ostream &out, const ExecResult& result) {
  out << "'" << result.stdout_str << "'";
  return out;
}


template <typename... Args>
ExecResult run(Args&&... args) {
  bp::ipstream my_stdout, my_stderr;
  bp::child x(std::forward<Args>(args)..., bp::std_out > my_stdout, bp::std_err > my_stderr);
  std::string output(std::string((std::istreambuf_iterator<char>(my_stdout)), std::istreambuf_iterator<char>()));
  std::string output2(std::string((std::istreambuf_iterator<char>(my_stderr)), std::istreambuf_iterator<char>()));
  x.wait();
  return ExecResult{x.exit_code(), std::move(output), std::move(output2)};
}

int main() {
  // auto env = boost::this_process::environment();
  // std::cout << run("echo -E xx\\nyy") << std::endl;
  // std::cout << run("g++ --version") << std::endl;
  // std::cout << run("uname -s") << std::endl;
  // std::cout << run("false") << std::endl;
  // std::cout << run("printenv", env) << std::endl;
  // std::cout << run("exit 4", env) << std::endl;
  auto adb_cmd = bp::search_path("adb");
  const char* ADB_ENV = std::getenv("ADB");
  if (ADB_ENV && std::strlen(ADB_ENV) > 0) {
    adb_cmd = ADB_ENV;
  }
  if (!exists(adb_cmd)) {
#ifdef __linux__
    adb_cmd = boost::filesystem::path(::getenv("HOME")) / "Android" / "Sdk" / "platform-tools" / "adb";
#endif
#ifdef __apple__
    adb_cmd = boost::filesystem::path(::getenv("HOME")) / "Library" / "Android" / "sdk" / "platform-tools" / "adb";
#endif
  }
  if (exists(adb_cmd) && is_regular_file(adb_cmd)) {
    auto x = run(adb_cmd, "devices");
    std::vector<std::string> devices;
    if (x.lines.size() > 1) {
      auto begin = x.lines.cbegin();
      std::advance(begin, 1);
      std::for_each(begin, x.lines.cend(), [&devices](const boost::string_ref& line) { devices.push_back(std::string(line.begin(), std::find_if(line.begin(), line.end(), [](char c1) { return std::isspace(c1); }))); });
    }

    for(const std::string& device: devices) {
      std::cout << "===============" << device << "==============" << std::endl;
      for (auto const& line :run(adb_cmd, "-s", device, "shell", "cmd", "package", "list", "packages", "-3").lines) {
        if (line.starts_with("package:")) {
          std::cout << "'" << boost::string_ref(line.begin() + 8, std::distance(line.begin() + 8, line.end())) << "'" <<std::endl;
        }
      }
      for (auto const& line :run(adb_cmd, "-s", device, "exec-out", "ps", "-A", "-w").lines) {
        if (line.starts_with("USER")) {
          continue;
        }
        std::vector<std::string> cols;
        boost::split(cols, line, boost::is_any_of(" \t"), boost::token_compress_on);
        if (cols.size() >= 2 && cols.rbegin()->size() > 0 && cols.rbegin()->data()[0] != '[') {
          std::cout << "'" << *cols.rbegin() << "'" << std::endl;
        }
      }
    }
  }
  return 0;
}

