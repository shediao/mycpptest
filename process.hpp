#ifndef _PROCESS_HPP_
#define _PROCESS_HPP_

#include <boost/process.hpp>
#include <iterator>
#include <string_view>
#include <vector>

class ProcessOutput {
public:
  ProcessOutput(int exit_code, std::vector<char> out, std::vector<char> err):
  _exit_code{exit_code},
  _stdout{std::move(out)},
  _stderr{std::move(err)}
  { }
  int exit_code() { return _exit_code; }
  const std::vector<char>& std_out() { return _stdout; }
  const std::vector<char>& std_err() { return _stderr; }
  std::string_view stdout_str() { return std::string_view(_stdout.data(), _stdout.size());}
  std::string_view stderr_str() { return std::string_view(_stderr.data(), _stderr.size());}
private:
  int _exit_code;
  std::vector<char> _stdout;
  std::vector<char> _stderr;
};

class ShellCommand {
public:
  enum class ShellType {
    BASH,
    ZSH,
    FISH,
  };
public:
  ShellType shell = ShellType::BASH;
  template <typename... Args>
  ProcessOutput operator()(Args&&... args) {

    boost::process::ipstream stdout_stream;
    boost::process::ipstream stderr_stream;

    boost::process::child child(
      boost::process::search_path("bash"),
      "-c",
      std::forward<Args>(args)...,
      boost::process::std_out > stdout_stream,
      boost::process::std_err > stderr_stream
    );

    std::vector<char> stdout_buf((std::istreambuf_iterator<char>(stdout_stream)), (std::istreambuf_iterator<char>()));
    std::vector<char> stderr_buf((std::istreambuf_iterator<char>(stderr_stream)), (std::istreambuf_iterator<char>()));
    int exit_code{-1};
    if (child.valid()) {
      child.wait();
      exit_code = child.exit_code();
    }
    return ProcessOutput(exit_code, std::move(stdout_buf), std::move(stderr_buf));
  }
};

namespace {
  ShellCommand $;
}
#endif // _PROCESS_HPP_

