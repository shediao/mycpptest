
CC := gcc
CXX := g++

HostOs := $(shell uname -s)

out_dir := $(shell uname -s)_$(shell uname -m)

BOOST_ROOT := /opt/boost/1.75.0
ifneq ("$(wildcard /opt/boost/1.76.0/lib)","")
  BOOST_ROOT := /opt/boost/1.76.0
endif
RANGE_V3_ROOT := ${HOME}/sources/range-v3
GTEST_ROOT := ${HOME}/sources/googletest/googletest

cflags := -Wall -Werror -I ${BOOST_ROOT}/include -O0 -g
cxxflags := -I ${RANGE_V3_ROOT}/include -I ${GTEST_ROOT}/include -std=c++17
ldflags := -pthread

libcxx_libs :=
ifneq ("$(wildcard /opt/libc++/12.0.0/lib/libc++.a)","")
  ldflags := ${ldflags} -nostdlib++
  libcxx_libs := /opt/libc++/12.0.0/lib/libc++.a /opt/libc++/12.0.0/lib/libc++abi.a
  cxxflags := ${cxxflags} -nostdinc++ -isystem/opt/libc++/12.0.0/include/c++/v1
  cflags := ${cflags} -Wno-deprecated-declarations
else
  ifeq ($(HostOs), Linux)
    ldflags := ${ldflags} -pthread -static-libstdc++ -static-libgcc
  endif
endif

BOOST_FILESYSTEM_LIB := ${BOOST_ROOT}/lib/libboost_filesystem.a

.PHONY: all clean run

all: ${out_dir}/process_test ${out_dir}/ranges_test ${out_dir}/filesystem_test ${out_dir}/adb_test ${out_dir}/adbauto ${out_dir}/smart_adb
run: all
	@${out_dir}/process_test
	@${out_dir}/filesystem_test
	@${out_dir}/ranges_test
	@${out_dir}/adb_test

${out_dir}:
	mkdir -p ${out_dir}

clean:
	find . -name '*.o' -delete

${out_dir}/gtest-all.o : ${GTEST_ROOT}/src/gtest-all.cc | ${out_dir}
	${CXX} ${cflags} ${cxxflags} -I ${GTEST_ROOT} -c $< -o $@
${out_dir}/gtest_main.o : ${GTEST_ROOT}/src/gtest_main.cc | ${out_dir}
	${CXX} ${cflags} ${cxxflags} -I ${GTEST_ROOT} -c $< -o $@

${out_dir}/process_test.o: process_test.cc | ${out_dir}
	${CXX} ${cflags} ${cxxflags} -c $< -o $@
${out_dir}/process_test: ${out_dir}/gtest-all.o ${out_dir}/process_test.o
	${CXX} ${ldflags} $^ -o $@ ${libcxx_libs} ${BOOST_FILESYSTEM_LIB}

${out_dir}/ranges_test.o: ranges_test.cc | ${out_dir}
	${CXX} ${cflags} ${cxxflags} -c $< -o $@
${out_dir}/ranges_test: ${out_dir}/ranges_test.o ${out_dir}/gtest-all.o | ${out_dir}
	${CXX} ${ldflags} $< -o $@ ${libcxx_libs}

${out_dir}/filesystem_test.o: filesystem_test.cc | ${out_dir}
	${CXX} ${cflags} ${cxxflags} -c $< -o $@
${out_dir}/filesystem_test: ${out_dir}/filesystem_test.o ${out_dir}/gtest-all.o ${out_dir}/gtest_main.o | ${out_dir}
	${CXX} ${ldflags} $^ -o $@  ${libcxx_libs} ${BOOST_FILESYSTEM_LIB}

${out_dir}/adb.o: adb.cc | ${out_dir}
	${CXX} ${cflags} ${cxxflags} -c $< -o $@

${out_dir}/adb_test.o: adb_test.cc | ${out_dir}
	${CXX} ${cflags} ${cxxflags} -c $< -o $@
${out_dir}/adb_test: ${out_dir}/gtest-all.o ${out_dir}/adb_test.o ${out_dir}/adb.o | ${out_dir}
	${CXX} ${ldflags} $^ -o $@  ${libcxx_libs} ${BOOST_FILESYSTEM_LIB}

${out_dir}/adbauto.o: adbauto.cc | ${out_dir}
	${CXX} ${cflags} ${cxxflags} -c $< -o $@

${out_dir}/adbauto: ${out_dir}/adbauto.o | ${out_dir}
	${CXX} ${ldflags} $^ -o $@ ${libcxx_libs} ${BOOST_FILESYSTEM_LIB}

${out_dir}/smart_adb.o: smart_adb.cc | ${out_dir}
	${CXX} ${cflags} ${cxxflags} -c $< -o $@
${out_dir}/smart_adb: ${out_dir}/smart_adb.o | ${out_dir}
	${CXX} ${ldflags} -o $@ $^ ${libcxx_libs} ${BOOST_FILESYSTEM_LIB}
