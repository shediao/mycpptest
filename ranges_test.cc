
#include <vector>
#include <string>
#include <iostream>
#include <range/v3/all.hpp>


template<typename T>
void print_vector(T&& t) {
  std::cout << "{ ";
  for (auto&& x: t) {
    std::cout << x << ", ";
  }
  std::cout << "}" << std::endl;
}

int main(int argc, const char* argv[]) {

    using namespace ranges;

    std::vector<int> x1{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    std::vector<std::string> y1 = x1
        | views::remove_if([](int x){ return (x % 2) == 0;})
        | views::transform([](int x){ return std::to_string(x);}) | to<std::vector>();

    print_vector(x1);
    print_vector(y1);


    print_vector(views::ints(1, unreachable) | views::transform([](int x){ return x*x;}) | views::take(10) | to<std::vector>);

    return 0;
}




