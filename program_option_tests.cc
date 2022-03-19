#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;
int main(int argc, const char* argv[]) {

  po::options_description desc("xxxxxxxxxx");
  desc.add_options()
    ("help", "帮助信息")
    ("compress", po::value<int>(), "压缩级别")
  ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc),vm);
  po::notify(vm);

  if (vm.count("help") > 0) {
    std::cout << desc << std::endl;
    return 1;
  }

  return 0;
}
