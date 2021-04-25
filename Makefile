
CC := gcc
CXX := g++

HostOs := $(shell uname)

BOOST_ROOT := /opt/boost/1.76.0

RANGE_V3_ROOT := ${HOME}/sources/range-v3

GTEST_ROOT := ${HOME}/sources/googletest/googletest

cflags := -Wall -Werror -I ${BOOST_ROOT}/include -O0 -g -Wno-deprecated-declarations
cxxflags := -I ${RANGE_V3_ROOT}/include -I ${GTEST_ROOT}/include -std=c++17 -nostdinc++ -isystem/opt/libc++/12.0.0/include/c++/v1

ldflags := -pthread

libcxx_libs :=
ifneq ("$(wildcard /opt/libc++/12.0.0/lib/libc++.a)","")
  ldflags := ${ldflags} -nostdlib++
  libcxx_libs := /opt/libc++/12.0.0/lib/libc++.a /opt/libc++/12.0.0/lib/libc++abi.a
else
  ifeq ($(HostOs), Linux)
    ldflags := ${ldflags} -pthread -static-libstdc++ -static-libgcc
  endif
endif

BOOST_FILESYSTEM_LIB := ${BOOST_ROOT}/lib/libboost_filesystem.a

.PHONY: all clean run

all: process_test ranges_test filesystem_test adb_test adbauto smart_adb
run: all
	./process_test
	./filesystem_test
	./ranges_test
	./adb_test


clean:
	find . -name '*.o' -delete

gtest-all.o : ${GTEST_ROOT}/src/gtest-all.cc
	${CXX} ${cflags} ${cxxflags} -I ${GTEST_ROOT} -c $< -o $@
gtest_main.o : ${GTEST_ROOT}/src/gtest_main.cc
	${CXX} ${cflags} ${cxxflags} -I ${GTEST_ROOT} -c $< -o $@

process_test.o: process_test.cc
	${CXX} ${cflags} ${cxxflags} -c $< -o $@
process_test: gtest-all.o process_test.o
	${CXX} ${ldflags} $^ -o $@ ${libcxx_libs} ${BOOST_FILESYSTEM_LIB}

ranges_test.o: ranges_test.cc
	${CXX} ${cflags} ${cxxflags} -c $< -o $@
ranges_test: ranges_test.o gtest-all.o
	${CXX} ${ldflags} $< -o $@ ${libcxx_libs}

filesystem_test.o: filesystem_test.cc
	${CXX} ${cflags} ${cxxflags} -c $< -o $@
filesystem_test: filesystem_test.o gtest-all.o gtest_main.o
	${CXX} ${ldflags} $^ -o $@  ${libcxx_libs} ${BOOST_FILESYSTEM_LIB}

adb.o: adb.cc
	${CXX} ${cflags} ${cxxflags} -c $< -o $@

adb_test.o: adb_test.cc
	${CXX} ${cflags} ${cxxflags} -c $< -o $@
adb_test: gtest-all.o adb_test.o ./adb.o
	${CXX} ${ldflags} $^ -o $@  ${libcxx_libs} ${BOOST_FILESYSTEM_LIB}

adbauto.o: adbauto.cc
	${CXX} ${cflags} ${cxxflags} -c $< -o $@

adbauto: adbauto.o
	${CXX} ${ldflags} $^ -o $@ ${libcxx_libs} ${BOOST_FILESYSTEM_LIB}

smart_adb.o: smart_adb.cc
	${CXX} ${cflags} ${cxxflags} -c $< -o $@
smart_adb: smart_adb.o
	${CXX} ${ldflags} -o $@ $^ ${libcxx_libs} ${BOOST_FILESYSTEM_LIB}
