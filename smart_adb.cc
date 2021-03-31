

#include <boost/process.hpp>

#include <iostream>
#include <iterator>
#include <string_view>

#include <range/v3/all.hpp>


// TODO:
// 1. command not exists
// 2. process ternimal
// 3. set environment
// 4. not capture stdout and stderr
// 5. close stdout,stderr,stdin
// 6. redectory stdout,stderr
class ChildProcess {
public:
  template <typename... Args>
  explicit ChildProcess(Args&&... args):
    stdout_stream(),
    stderr_stream(),
    child(std::forward<Args>(args)..., boost::process::std_out > stdout_stream, boost::process::std_err > stderr_stream),
    stdout_buf(std::istreambuf_iterator<char>(stdout_stream), std::istreambuf_iterator<char>()),
    stderr_buf(std::istreambuf_iterator<char>(stderr_stream), std::istreambuf_iterator<char>()),
    _exit_code(-1),
    _all_lines_range(stdout_buf | ranges::views::trim((int(*)(int))&std::isspace) | ranges::views::split_when(&isnewline))
    { if (child.valid()) { child.wait(); _exit_code = child.exit_code(); } }

  ChildProcess() = delete;
  ChildProcess(const ChildProcess&) = delete;
  ChildProcess& operator=(const ChildProcess&) = delete;

  ChildProcess(ChildProcess&&) = delete;
  ChildProcess& operator=(ChildProcess&&) = delete;

  std::string_view stdout_str() { return std::string_view(stdout_buf.data(), stdout_buf.size()); }
  std::string_view stderr_str() { return std::string_view(stderr_buf.data(), stderr_buf.size()); }
  const std::vector<char>& stdout_raw() { return stdout_buf; }
  const std::vector<char>& stderr_raw() { return stderr_buf; }
  std::string_view first_line() & {
    decltype(auto) first_line_range = *_all_lines_range.begin();
    return std::string_view(&*(first_line_range.begin()), ranges::distance(first_line_range));
  }
  std::string first_line() && {
    decltype(auto) first_line_range = *_all_lines_range.begin();
    return std::string(&*(first_line_range.begin()), ranges::distance(first_line_range));
  }
  int exit_code() { return _exit_code; }
  auto& all_lines_range() {
    return this->_all_lines_range;
  }
private:
  static bool isnewline(char x) { return x == '\n' || x == '\r'; }
private:
  boost::process::ipstream stdout_stream;
  boost::process::ipstream stderr_stream;
  boost::process::child child;
  std::vector<char> stdout_buf;
  std::vector<char> stderr_buf;
  int _exit_code;
  decltype(stdout_buf | ranges::views::trim((int(*)(int))nullptr) | ranges::views::split_when((bool(*)(char))nullptr)) _all_lines_range;
};

class Run {
public:
  template <typename... Args>
  explicit Run(Args&&... args):child(std::forward<Args>(args)...), _exit_code(-1) {
    if (child.valid()) {
      child.wait();
      _exit_code = child.exit_code();
    }
  }

  Run() = delete;
  Run(const Run&) = delete;
  Run& operator=(const Run&) = delete;

  Run(Run&&) = delete;
  Run& operator=(Run&&) = delete;

  int exit_code() { return _exit_code; }
private:
  boost::process::child child;
  int _exit_code;
};

auto get_adb_path() {
  auto adb_cmd = boost::process::search_path("adb");
  if (exists(adb_cmd)) { return adb_cmd; }

  const char* adb_env = std::getenv("ADB");
  if (adb_env && std::strlen(adb_env) > 0) { adb_cmd = adb_env; }
  if (exists(adb_cmd)) { return adb_cmd; }

  for (auto env : {"ANDROID_SDK_ROOT", "ANDROID_SDK", "ANDROID_HOME"}) {
    const char* my_env = std::getenv(env);
    if (my_env && std::strlen(my_env) > 0) {
        adb_cmd = my_env;
        adb_cmd = adb_cmd / "platform-tools" / "adb";
    }
    if (exists(adb_cmd)) { return adb_cmd; }
  }
#ifdef __linux__
  adb_cmd = boost::filesystem::path(::getenv("HOME")) / "Android" / "Sdk" / "platform-tools" / "adb";
#endif
#ifdef __apple__
  adb_cmd = boost::filesystem::path(::getenv("HOME")) / "Library" / "Android" / "sdk" / "platform-tools" / "adb";
#endif
  if (exists(adb_cmd)) { return adb_cmd; }

  return boost::filesystem::path("");
}

int main(int argc, char* argv[]) {
  auto adb_path = get_adb_path();
  if (exists(adb_path)) {
    return ::Run(adb_path, "--version").exit_code();
  }
  return 1;
}

