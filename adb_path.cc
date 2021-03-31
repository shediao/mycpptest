#include <boost/process.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>

#include <iostream>
#include <string_view>
#include <range/v3/all.hpp>

#include "adb_path.h"



namespace {
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

  ChildProcess(ChildProcess&&) = default;
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
  auto& all_lines_range() & {
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
}


std::vector<Adb> GetAllAdbFromPath() {
  std::vector<Adb> adb_path;
  std::vector<boost::filesystem::path> path = ::boost::this_process::path();

  for (const boost::filesystem::path & pp : path) {
    if (pp.leaf() == "buildtools") { continue; }
    auto p = pp / "adb";
    boost::system::error_code ec;
    bool file = boost::filesystem::is_regular_file(p, ec);
    if (!ec && file && ::access(p.c_str(), X_OK) == 0) {
      adb_path.push_back(Adb(p));
    }
  }
  return adb_path;
}

std::vector<Adb> GetAllAdbs() {

  std::vector<Adb> adb_path;
  auto home_dir = boost::filesystem::path(std::getenv("HOME"));
#ifdef __linux__
  auto default_android_sdk = home_dir / "Android" / "Sdk";
#endif  // __linux__
#ifdef __APPLE__
  auto default_android_sdk = home_dir / "Library" / "Android" / "sdk";
#endif  // __APPLE__
  std::vector<boost::filesystem::path> search_path;
  search_path.push_back(default_android_sdk / "platform-tools");

  for (auto env: {"ANDROID_SDK_ROOT", "ANDROID_HOME"}) {
    const char* env_value = std::getenv(env);
    if (env_value && std::strlen(env_value)) {
      search_path.push_back(boost::filesystem::path(env_value) / "platform-tools");
    }
  }

  for (const boost::filesystem::path & pp : search_path) {
    if (pp.leaf() != "platform-tools") { continue; }
    auto p = pp / "adb";
    boost::system::error_code ec;
    bool file = boost::filesystem::is_regular_file(p, ec);
    if (!ec && file && ::access(p.c_str(), X_OK) == 0) {
      adb_path.push_back(Adb(p));
    }
  }

  auto adbsFromPath = GetAllAdbFromPath();
  adb_path.insert(adb_path.end(), adbsFromPath.begin(), adbsFromPath.end());
  return adb_path;
}

Adb GetDefaultAdb() {
  auto all_adbs = GetAllAdbs();
  if (all_adbs.size() > 0 ) {
    return all_adbs[0];
  }
  return Adb(boost::filesystem::path(""));
}

Adb GetNewestAdb() {
  auto all_adbs = GetAllAdbs();
  if (all_adbs.size() > 0 ) {
    std::sort(all_adbs.begin(), all_adbs.end(), [](Adb& l, Adb& r) { return l.version() > r.version(); } );
    return all_adbs[0];
  }
  return Adb(boost::filesystem::path(""));
}
Adb GetAdbFromPath() {
  std::vector<boost::filesystem::path> path = ::boost::this_process::path();

  for (const boost::filesystem::path & pp : path) {
    if (pp.leaf() == "buildtools") { continue; }
    auto p = pp / "adb";
    boost::system::error_code ec;
    bool file = boost::filesystem::is_regular_file(p, ec);
    if (!ec && file && ::access(p.c_str(), X_OK) == 0) {
      return Adb(p);
    }
  }
  return Adb(boost::filesystem::path(""));
}


int GetAdbdVersion(int port) {
  int version = -1;
  try {
    using boost::asio::ip::tcp;
    boost::asio::io_service io;
    tcp::socket socket(io);
    tcp::endpoint endpoint(boost::asio::ip::address::from_string("127.0.0.1"), port);
    socket.connect(endpoint);

    // write
    std::string version_msg {"000Chost:version"};
    boost::system::error_code ignored_error;
    boost::asio::write(socket, boost::asio::buffer(version_msg), ignored_error);

    // read
    boost::array<char, 128> buf;
    std::vector<char> response;
    boost::system::error_code error;
    size_t len = socket.read_some(boost::asio::buffer(buf), error);
    while (len > 0) {
      if (error) return version;
      response.insert(response.end(), buf.data(), buf.data() + len);
      len = socket.read_some(boost::asio::buffer(buf), error);
    }

    std::string_view result(response.data(), response.size());

    std::string_view start = "OKAY0004";
    if (result.find(start.data()) == 0) {
      result = result.substr(start.size());
      version = std::stoi(std::string(result.data(), result.length()), 0, 16);
    } else {
      std::cerr << "adbd response:" << result << std::endl;
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    version = -1;
  }
  return version;
}

int Adb::version() {
  if (adb_version > 0) { return adb_version; }
  std::string_view prefix{"Android Debug Bridge version"};
  ChildProcess adb_version_process(adb_path, "--version");
  auto x{adb_version_process.first_line()};
  if (0 == x.find(prefix.data())) {
    x = x.substr(prefix.length());
    auto versions = x | ranges::views::trim((int(*)(int))&std::isspace) | ranges::views::split_when([](int x){ return x == '.';}) | ranges::to<std::vector<std::string>>;
    if (versions.size() == 3) {
      return std::stoi(versions[2]);
    } else {
      adb_version = -1;
    }
  } else {
    adb_version = -1;
  }
  return adb_version;
}

std::vector<std::string> Adb::devices() {
  ChildProcess adb_version_process(adb_path, "devices", "-l");
  auto all_lines = adb_version_process.all_lines_range()
    | ranges::to<std::vector<std::string>>;

  std::vector<std::string> android_devices;
  if (all_lines.size() == 0 || all_lines[0].find("List of devices attached") != 0) {
    return android_devices;
  }
  for (auto const& d: all_lines | ranges::views::drop(1)) {
    auto x = d | ranges::views::split_when((int(*)(int))&std::isspace) | ranges::to<std::vector<std::string>>;
    if (x.size() > 1) {
      android_devices.push_back(x[0]);
    }
  }
  return android_devices;
}

