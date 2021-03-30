#include <iostream>
#include <ranges>
#include <algorithm>

namespace rv = std::ranges::views;

int main()
{
    int x = 0;
        auto is_even = [&x](int const n) {++x; return n % 2 == 0;};
            
            std::vector<int> n{1,1,2,3,5,8,13,21,34,55,89,144,233,377};
                auto v = n | rv::filter(is_even) | rv::reverse | rv::take(2) ;
                    std::ranges::copy(v, std::ostream_iterator<int>(std::cout, " "));
    std::cout << "cout:" << x << std::endl;
}
