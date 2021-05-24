

#include <boost/process.hpp>

#include <iostream>
#include <iterator>
#include <string_view>

#include <range/v3/all.hpp>

#include <gtest/gtest.h>

#include "process.hpp"

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

TEST(CommandPath, CheckExists) {
  ASSERT_TRUE(exists(boost::process::search_path("echo")));
  ASSERT_TRUE(exists(boost::process::search_path("true")));
  ASSERT_TRUE(exists(boost::process::search_path("false")));
  ASSERT_FALSE(exists(boost::process::search_path("echoxxxxxxxxxxxxxxxxxxxxxxxxxx")));
}

TEST(ChildProcess, ExitCode) {
  ASSERT_EQ(ChildProcess(boost::process::search_path("true")).exit_code(), 0);
  ASSERT_EQ(ChildProcess(boost::process::search_path("false")).exit_code(), 1);
  ASSERT_EQ(ChildProcess(boost::process::search_path("bash"), "-c", "exit 0").exit_code(), 0);
  ASSERT_EQ(ChildProcess(boost::process::search_path("bash"), "-c", "exit 1").exit_code(), 1);
  ASSERT_EQ(ChildProcess(boost::process::search_path("bash"), "-c", "exit 2").exit_code(), 2);
  ASSERT_EQ(ChildProcess(boost::process::search_path("bash"), "-c", "exit 128").exit_code(), 128);
  ASSERT_EQ(ChildProcess(boost::process::search_path("bash"), "-c", "exit 255").exit_code(), 255);
  ASSERT_EQ(ChildProcess(boost::process::search_path("bash"), "-c", "exit 256").exit_code(), 0);
}

TEST(ChildProcess, Stdout) {
  ASSERT_EQ(ChildProcess("echo 123").stdout_str(), "123\n");
  ASSERT_EQ(ChildProcess("echo -n 123").stdout_str(), "123");
  ASSERT_EQ(ChildProcess("echo -n 1\n2\n3").stdout_str(), "1\n2\n3");
  ASSERT_EQ(ChildProcess("echo -n \n\n\n\n\n").stdout_str(), "\n\n\n\n\n");
  ASSERT_EQ(ChildProcess("echo -n \r\r\r\r\r").stdout_str(), "\r\r\r\r\r");
  ASSERT_EQ(ChildProcess("echo -n '      '").stdout_str(), "' '");
  ASSERT_EQ(ChildProcess("echo").stdout_str(), "\n");
  ASSERT_EQ(ChildProcess(boost::process::search_path("echo"), boost::process::args={"1", "2", "3"}).stdout_str(), "1 2 3\n");
  ASSERT_EQ(ChildProcess(boost::process::search_path("echo"), "123").stdout_str(), "123\n");
  ASSERT_EQ(ChildProcess(boost::process::search_path("echo"), "-n", "123").stdout_str(), "123");
  ASSERT_EQ(ChildProcess(boost::process::search_path("echo"), "-n", "1\n2\n3\n").stdout_str(), "1\n2\n3\n");
  ASSERT_EQ(ChildProcess(boost::process::search_path("echo"), "1\n2\n3\n").stdout_str(), "1\n2\n3\n\n");
}

TEST(ChildProcess, FirstLine) {
  ASSERT_EQ(ChildProcess(boost::process::search_path("echo")).first_line(), "");
  ASSERT_EQ(ChildProcess("echo 123").first_line(), "123");
  ASSERT_EQ(ChildProcess("echo \n123").first_line(), "123");
  ASSERT_EQ(ChildProcess("echo \n123\n\n").first_line(), "123");
  ASSERT_EQ(ChildProcess("echo \n\n\n\n\n\n\n123").first_line(), "123");
}

TEST(Run, ExitCode) {
  ASSERT_EQ(::Run(boost::process::search_path("true")).exit_code(), 0);
  ASSERT_EQ(::Run(boost::process::search_path("false")).exit_code(), 1);

  ASSERT_EQ(::Run(boost::process::search_path("echo"), "1 2 3", boost::process::std_out > boost::process::null).exit_code(), 0);

  ASSERT_EQ(::Run(boost::process::search_path("bash"), "-c", "exit 0").exit_code(), 0);
  ASSERT_EQ(::Run(boost::process::search_path("bash"), "-c", "exit 1").exit_code(), 1);
  ASSERT_EQ(::Run(boost::process::search_path("bash"), "-c", "exit 255").exit_code(), 255);
  ASSERT_EQ(::Run(boost::process::search_path("bash"), "-c", "exit 256").exit_code(), 0);


  ASSERT_EQ(::Run(boost::process::search_path("bash"), boost::process::args={"-c", "exit 1"}).exit_code(), 1);
  ASSERT_EQ(::Run(boost::process::search_path("echo"), "1", boost::process::args+={"2", "3"}).exit_code(), 0);


  // ASSERT_EQ(::Run(boost::process::search_path("sleep"), "3").exit_code(), 0);

}

TEST(Doler, Run) {
  auto x = $("echo -n 123");
  ASSERT_EQ(x.stdout_str(), "123");
}

int main(int argc, char* argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

