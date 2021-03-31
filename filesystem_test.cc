
#include <iostream>
#include <boost/filesystem.hpp>
#include <range/v3/all.hpp>

#include <gtest/gtest.h>



using namespace boost::filesystem;

TEST(Filesystem, Path) {
  path home_dir{std::getenv("HOME")};
  ASSERT_TRUE(exists(home_dir));
  ASSERT_TRUE(exists(home_dir / "sources"));
  std::vector<std::string> files;
  for (const auto& x: recursive_directory_iterator(home_dir / "sources" / "mycpptest")) {
    files.push_back(x.path().string());
  }
  std::cout << files.size() << std::endl;
  std::cout << std::getenv("PATH") << std::endl;


  // std::vector x{1, 2, 3, 4};

  // for (const auto& x : std::string_view(std::getenv("PATH")) | ranges::views::split_when([](int x){ return x == ':';})) {
  //     std::cout << std::string_view(&*x.begin(), ranges::distance(x)) << std::endl;
  // }
}

