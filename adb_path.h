
#ifndef ADB_PATH
#define ADB_PATH
#include <boost/filesystem.hpp>

class Adb {
 public:
  struct AdbShellResult {
    int exit_code = 0;
    std::string std_out;
    std::string std_err;
  };
 public:
  Adb(boost::filesystem::path adb):adb_path(adb){ }

  int version();

  int push(const char* src, const char* dst);
  int pull(const char* src, const char* dst);
  int install(boost::filesystem::path const& apk_file);
  std::vector<std::string> devices();
  // template <typename... Args>
  // ChildProcess shell(Args&&... args) { return ChildProcess(adb_path, "shell", std::forward<Args>(args)...);  }
  AdbShellResult shell() { return AdbShellResult{}; }
  int shell_run() { return 1; }

  const boost::filesystem::path& path() {
    return adb_path;
  }

  void set_execute_path(const boost::filesystem::path adb) { adb_path = adb; }

 private:
  boost::filesystem::path adb_path;
  int adb_version = -1;
};

int GetAdbdVersion();
Adb GetDefaultAdb();
Adb GetNewestAdb();
Adb GetAdbFromPath();
std::vector<Adb> GetAllAdbs();



#endif  // ADB_PATH
