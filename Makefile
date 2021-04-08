
CC := gcc
CXX := g++

HostOs := $(shell uname)

BOOST_ROOT := /opt/boost/1.75.0

RANGE_V3_ROOT := ${HOME}/sources/range-v3

GTEST_ROOT := ${HOME}/sources/googletest/googletest

cflags := -Wall -Werror -I ${BOOST_ROOT}/include -O0 -g
cxxflags := -I ${RANGE_V3_ROOT}/include -I ${GTEST_ROOT}/include -std=c++17

ifeq ($(HostOs),Darwin)
  ldflags := -pthread
else
  ldflags := -pthread -static-libstdc++ -static-libgcc
endif

static_libs := ${BOOST_ROOT}/lib/libboost_filesystem.a


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
	${CXX} ${ldflags} $^ -o $@  ${static_libs}

ranges_test.o: ranges_test.cc
	${CXX} ${cflags} ${cxxflags} -c $< -o $@
ranges_test: ranges_test.o gtest-all.o
	${CXX} ${ldflags} $< -o $@

filesystem_test.o: filesystem_test.cc
	${CXX} ${cflags} ${cxxflags} -c $< -o $@
filesystem_test: filesystem_test.o gtest-all.o gtest_main.o
	${CXX} ${ldflags} $^ -o $@  ${static_libs}

adb.o: adb.cc
	${CXX} ${cflags} ${cxxflags} -c $< -o $@

adb_test.o: adb_test.cc
	${CXX} ${cflags} ${cxxflags} -c $< -o $@
adb_test: gtest-all.o adb_test.o ./adb.o
	${CXX} ${ldflags} $^ -o $@  ${static_libs}

adbauto.o: adbauto.cc
	${CXX} ${cflags} ${cxxflags} -c $< -o $@

adbauto: adbauto.o
	${CXX} ${ldflags} $^ -o $@ ${static_libs}

smart_adb.o: smart_adb.cc
	${CXX} ${cflags} ${cxxflags} -c $< -o $@
smart_adb: smart_adb.o
	${CXX} ${ldflags} -o $@ $^ ${static_libs}
