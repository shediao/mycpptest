
#ifndef ADB_PATH
#define ADB_PATH
#include <boost/filesystem.hpp>

boost::filesystem::path get_adb_path();

class Adb {
 public:
  Adb():adb_path(get_adb_path()){ }
  Adb(boost::filesystem::path adb):adb_path(adb){ }

  int version();

  int push(const char* src, const char* dst);
  int pull(const char* src, const char* dst);
  int install(boost::filesystem::path const& apk_file);
  std::vector<std::string> devices();
  // template <typename... Args>
  // ChildProcess shell(Args&&... args) { return ChildProcess(adb_path, "shell", std::forward<Args>(args)...);  }
  int run() { return 1; }

  const boost::filesystem::path& path() {
    return adb_path;
  }

 private:
  boost::filesystem::path adb_path;
};



#endif  // ADB_PATH
