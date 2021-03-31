

# CC := clang-11
# CXX := clang++-11
# CC := gcc-10
# CXX := g++-10
CC := clang
CXX := clang++

BOOST_ROOT := /opt/boost/1.75.0

RANGE_V3_ROOT := ${HOME}/sources/range-v3

GTEST_ROOT := ${HOME}/sources/googletest/googletest

cflags := -Wall -Werror -I ${BOOST_ROOT}/include -g2 -O0
cxxflags := -I ${RANGE_V3_ROOT}/include -I ${GTEST_ROOT}/include -std=c++17

ldflags := -pthread

static_libs := ${BOOST_ROOT}/lib/libboost_filesystem.a


.PHONY: all clean run

run: process_test ranges_test filesystem_test adb_test
	./process_test
	./filesystem_test
	./ranges_test
	./adb_test

all: process_test ranges_test filesystem_test

clean:
	find . -name '*.o' -delete

gtest-all.o : ${GTEST_ROOT}/src/gtest-all.cc
	${CXX} ${cflags} ${cxxflags} -I ${GTEST_ROOT} -c $< -o $@
gtest_main.o : ${GTEST_ROOT}/src/gtest_main.cc
	${CXX} ${cflags} ${cxxflags} -I ${GTEST_ROOT} -c $< -o $@

process_test.o: process_test.cc
	${CXX} ${cflags} ${cxxflags} -c $< -o $@
process_test: gtest-all.o process_test.o
	${CXX} ${ldflags} $^ -o $@  ${static_libs}

ranges_test.o: ranges_test.cc
	${CXX} ${cflags} ${cxxflags} -c $< -o $@
ranges_test: ranges_test.o gtest-all.o
	${CXX} ${ldflags} $< -o $@

filesystem_test.o: filesystem_test.cc
	${CXX} ${cflags} ${cxxflags} -c $< -o $@
filesystem_test: filesystem_test.o gtest-all.o gtest_main.o
	${CXX} ${ldflags} $^ -o $@  ${static_libs}

adb_path.o: adb_path.cc
	${CXX} ${cflags} ${cxxflags} -c $< -o $@

adb_test.o: adb_test.cc
	${CXX} ${cflags} ${cxxflags} -c $< -o $@
adb_test: gtest-all.o adb_test.o ./adb_path.o
	${CXX} ${ldflags} $^ -o $@  ${static_libs}
