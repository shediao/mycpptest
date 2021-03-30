cmake_minimum_required(VERSION 3.16)
project(my_tests)


set(CMAKE_C_COMPILER gcc-10)
set(CMAKE_CXX_COMPILER g++-10)

set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -O0 -g -Wall -Werror -pthread)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -std=c++20)

set(BOOST_ROOT /opt/boost/1.75.0)
set(Boost_USE_STATIC_LIBS YES)

find_package(Boost REQUIRED COMPONENTS filesystem)


set(threads_spec ${CMAKE_THREAD_LIBS_INIT})

include_directories(${Boost_INCLUDE_DIRS} /home/shediao/sources/range-v3/include)

add_library(gtest_static STATIC /home/shediao/sources/googletest/googletest/src/gtest-all.cc)
add_library(gmock_static STATIC /home/shediao/sources/googletest/googlemock/src/gmock-all.cc)

target_include_directories(gtest_static PRIVATE /home/shediao/sources/googletest/googletest)
target_include_directories(gmock_static PRIVATE /home/shediao/sources/googletest/googlemock)

target_include_directories(gtest_static PUBLIC /home/shediao/sources/googletest/googletest/include)
target_include_directories(gmock_static PUBLIC /home/shediao/sources/googletest/googlemock/include)

target_link_libraries(gtest_static  pthread ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(gmock_static gtest_static)

add_executable(process_test process_test.cc)

target_link_libraries(process_test gtest_static ${Boost_LIBRARIES})

