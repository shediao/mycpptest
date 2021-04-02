#include <iostream>
#include <array>

#include <boost/process.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <range/v3/all.hpp>

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
            std::vector<std::string> replace_commands;
            for (auto i = std::next(commands.begin(), 1); i != commands.end(); ++i) {
              replace_commands.push_back(boost::algorithm::replace_all_copy(*i, "{}", serial));
            }
            boost::process::child child(boost::process::search_path(commands[0]), boost::process::args=replace_commands);
            child.detach();
          }
        }
      }
    }
  }
}

void TrackAndroidDevice(int port, std::vector<std::string> const& commands) {
  try {
    using boost::asio::ip::tcp;
    boost::asio::io_service io;
    tcp::socket socket(io);
    tcp::endpoint endpoint(boost::asio::ip::address::from_string("127.0.0.1"), port);
    socket.connect(endpoint);

    // write
    std::string_view request_msg("0012host:track-devices");
    boost::system::error_code ignored_error;
    boost::asio::write(socket, boost::asio::buffer(request_msg), ignored_error);

    // read
    std::array<char, 128> buf;
    std::vector<char> response_data;
    boost::system::error_code error;
    // TODO: boost::asio::read
    std::size_t read_len = boost::asio::read(socket, boost::asio::buffer(buf), boost::asio::transfer_at_least(4), error);
    if (error) {
      return;
    }

    if (std::string_view(buf.data(), 4) != "OKAY" ) {
      std::cerr << "adbd response_data:" << std::string_view(buf.data(), buf.size()) << std::endl;
      return;
    }

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
    std::cerr << e.what() << std::endl;
  }
}

int main(int argc, const char* argv[]) {
  std::vector<std::string> commands;
  if (argc >= 2) {
    commands.insert(commands.end(), argv +1, argv + argc);
  }
  TrackAndroidDevice(5037, commands);
  return 0;
}
