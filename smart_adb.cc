

#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/process/environment.hpp>

#if __cpulsplus >= 201703
#include <string_view>
#else
#include <boost/utility/string_ref.hpp>
#endif


#include <iostream>
#include <iterator>
#include <array>


int GetAdbdVersion(std::string const& host, int port) {
  int version = -1;
  try {
    using boost::asio::ip::tcp;
    boost::asio::io_service io;
    tcp::socket socket(io);
    tcp::endpoint endpoint(boost::asio::ip::address::from_string(host), port);
    socket.connect(endpoint);

    // write
    std::string version_msg {"000Chost:version"};
    boost::system::error_code ignored_error;
    boost::asio::write(socket, boost::asio::buffer(version_msg), ignored_error);

    // read
    std::array<char, 64> buf;
    std::vector<char> response_data;
    boost::system::error_code error;
    auto read_len = boost::asio::read(socket, boost::asio::buffer(buf), boost::asio::transfer_at_least(8), error);
    if (error) {
      return -1;
    }

#if __cpulsplus >= 201703
    using string_view = std::string_view;
#else
    using string_view = boost::string_ref;
#endif
    if (string_view(buf.data(), 4) != "OKAY" ) {
      std::cerr << "adbd response_data:" << string_view(buf.data(), buf.size()) << std::endl;
      return -1;
    }
    int data_len = std::stoi(std::string(buf.data() + 4, 4), 0, 16);

    response_data.insert(response_data.end(), buf.data() + 8, buf.data() + read_len);

    while ((read_len = socket.read_some(boost::asio::buffer(buf), error)) > 0) {
      if (error) return -1;
      response_data.insert(response_data.end(), buf.data(), buf.data() + read_len);
      if (static_cast<int>(response_data.size()) >= data_len) { break; }
    }
    return std::stoi(std::string(response_data.data(), response_data.size()), 0, 16);
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    version = -1;
  }
  return version;
}

bool is_executable(const boost::filesystem::path &p) {
  boost::system::error_code ec;
  bool file = boost::filesystem::is_regular_file(p, ec);
  if (!ec && file && ::access(p.c_str(), X_OK) == 0) {
    return true;
  }
  return false;
}

boost::filesystem::path get_adb_path(boost::filesystem::path const& this_dir, int adb_server_version) {
  auto adb_cmd = boost::filesystem::path("");

  if (adb_server_version > 0) {
    adb_cmd = this_dir / "adbs" / ("adb-1.0." + std::to_string(adb_server_version));
    if (is_executable(adb_cmd)) { return adb_cmd; }
  }
#ifdef __linux__
  adb_cmd = boost::filesystem::path(::getenv("HOME")) / "Android" / "Sdk" / "platform-tools" / "adb";
#endif
#ifdef __APPLE__
  adb_cmd = boost::filesystem::path(::getenv("HOME")) / "Library" / "Android" / "sdk" / "platform-tools" / "adb";
#endif
  if (is_executable(adb_cmd)) { return adb_cmd; }

  std::vector<boost::filesystem::path> path = ::boost::this_process::path();
  for (const boost::filesystem::path & pp : path) {
    if (pp.leaf() == "platform-tools") {
      auto p = pp / "adb";
      if (is_executable(p) && is_executable( pp / "fastboot")) { return p; }
    }
  }

  for (auto env : {"ANDROID_SDK_ROOT", "ANDROID_SDK", "ANDROID_HOME"}) {
    const char* my_env = std::getenv(env);
    if (my_env && std::strlen(my_env) > 0) {
        adb_cmd = my_env;
        adb_cmd = adb_cmd / "platform-tools" / "adb";
    }
    if (is_executable(adb_cmd)) { return adb_cmd; }
  }

  adb_cmd = this_dir / "adb-last";
  return adb_cmd;
}

int main(int argc, char* argv[]) {
  int adb_server_version = GetAdbdVersion("127.0.0.1", 5037);
  boost::filesystem::path this_path(argv[0]);
  auto this_dir = this_path.parent_path();
  auto adb_path = get_adb_path(this_dir, adb_server_version);
  execv(boost::filesystem::absolute(adb_path).c_str(), argv);
  return 1;
}

