
#ifndef ADB_PATH
#define ADB_PATH
#include <boost/filesystem.hpp>
#include <boost/process.hpp>

struct AdbShellResult {
  std::string std_out;
  std::string std_err;
  int exit_code = 0;
};
class Adb {
 public:
  explicit Adb(boost::filesystem::path adb):adb_path(adb){ }


  void start_server();
  void kill_server();
  void forward();
  void get_stat();
  void get_serialno();

  void connect(std::string device);
  void disconnect(std::string device);

  int exec_out();
  int run_as();

  int version();

  int push(const char* src, const char* dst) {
    int exit_code = 1;
    boost::process::child child(adb_path, "push", src, dst);
    if (child.valid()) { child.wait(); exit_code = child.exit_code(); }
    return exit_code;
  }
  int pull(const char* src, const char* dst) {
    int exit_code = 1;
    boost::process::child child(adb_path, "pull", src, dst);
    if (child.valid()) { child.wait(); exit_code = child.exit_code(); }
    return exit_code;
  }
  int install(boost::filesystem::path const& apk_file) {
    int exit_code = 1;
    boost::process::child child(adb_path, "install", "-r",  apk_file.c_str());
    if (child.valid()) { child.wait(); exit_code = child.exit_code(); }
    return exit_code;
  }
  int uninstall(std::string package) {
    int exit_code = 1;
    boost::process::child child(adb_path, "uninstall", package.c_str());
    if (child.valid()) { child.wait(); exit_code = child.exit_code(); }
    return exit_code;
  }

  std::vector<std::string> devices();
  template <typename... Args>
  AdbShellResult shell(Args&&... args) {

    boost::process::ipstream stdout_stream;
    boost::process::ipstream stderr_stream;
    int exit_code = 1;

    boost::process::child child(adb_path, "exec-out", std::forward<Args>(args)..., boost::process::std_out > stdout_stream, boost::process::std_err > stderr_stream );
    std::string stdout_buf((std::istreambuf_iterator<char>(stdout_stream)), std::istreambuf_iterator<char>());
    std::string stderr_buf((std::istreambuf_iterator<char>(stderr_stream)), std::istreambuf_iterator<char>());

    if (child.valid()) { child.wait(); exit_code = child.exit_code(); }

    return AdbShellResult{ std::move(stdout_buf), std::move(stderr_buf), exit_code};
  }

  int shell_run() { return 1; }
  const boost::filesystem::path& path() { return adb_path; }
  void set_execute_path(const boost::filesystem::path& adb) { adb_path = adb; }
  void set_device(std::string device) { serial = std::move(device); }

 private:
  boost::filesystem::path adb_path;
  int adb_version = -1;
  std::string serial;
  int port = 5037;
};

int GetAdbdVersion(int port);
Adb GetDefaultAdb();
Adb GetNewestAdb();
Adb GetAdbFromPath();
std::vector<Adb> GetAllAdbs();

#endif  // ADB_PATH
