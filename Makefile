
LLVM_ROOT := /opt/llvm/11.x

HostOs := $(shell uname -s)

ifeq ($(HostOs), Linux)
CC := ${LLVM_ROOT}/bin/clang
CXX := ${LLVM_ROOT}/bin/clang++
else
CC := gcc #${LLVM_ROOT}/bin/clang
CXX := g++ # ${LLVM_ROOT}/bin/clang++
endif


out_dir := out/$(shell uname -s | tr A-Z a-z)-$(shell uname -m)

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

ifeq ($(HostOs), Linux)
    ldflags := ${ldflags} -static-libstdc++ -static-libgcc
else
  ifneq ("$(wildcard ${LLVM_ROOT}/lib/libc++.a)","")
    ldflags := ${ldflags} -nostdlib++
    libcxx_libs := ${LLVM_ROOT}/lib/libc++.a ${LLVM_ROOT}/lib/libc++abi.a
    cxxflags := -nostdinc++ -isystem${LLVM_ROOT}/include/c++/v1 ${cxxflags}
    cflags := ${cflags} -Wno-deprecated-declarations
  endif
endif

BOOST_LIBS := ${BOOST_ROOT}/lib/libboost_filesystem.a ${BOOST_ROOT}/lib/libboost_program_options.a

.PHONY: all clean run

all: ${out_dir}/process_test ${out_dir}/ranges_test ${out_dir}/filesystem_test ${out_dir}/adb_test ${out_dir}/adbauto ${out_dir}/smart_adb
run: all
	@${out_dir}/process_test
	@${out_dir}/filesystem_test
	@${out_dir}/ranges_test
	@${out_dir}/adb_test

${out_dir}:
	mkdir -p ${out_dir}

${out_dir}/%.o: %.cc | ${out_dir} Makefile
	${CXX} ${cflags} ${cxxflags} -I ${GTEST_ROOT} -c $< -o $@

clean:
	rm -rf ${out_dir}

${out_dir}/gtest-all.o : ${GTEST_ROOT}/src/gtest-all.cc | ${out_dir}
	${CXX} ${cflags} ${cxxflags} -I ${GTEST_ROOT} -c $< -o $@
${out_dir}/gtest_main.o : ${GTEST_ROOT}/src/gtest_main.cc | ${out_dir}
	${CXX} ${cflags} ${cxxflags} -I ${GTEST_ROOT} -c $< -o $@

${out_dir}/process_test: ${out_dir}/gtest-all.o ${out_dir}/process_test.o
	${CXX} ${ldflags} $^ -o $@ ${libcxx_libs} ${BOOST_LIBS}

${out_dir}/ranges_test: ${out_dir}/ranges_test.o ${out_dir}/gtest-all.o
	${CXX} ${ldflags} $< -o $@ ${libcxx_libs}

${out_dir}/filesystem_test: ${out_dir}/filesystem_test.o ${out_dir}/gtest-all.o ${out_dir}/gtest_main.o
	${CXX} ${ldflags} $^ -o $@  ${libcxx_libs} ${BOOST_LIBS}

${out_dir}/adb_test: ${out_dir}/gtest-all.o ${out_dir}/adb_test.o ${out_dir}/adb.o
	${CXX} ${ldflags} $^ -o $@  ${libcxx_libs} ${BOOST_LIBS}

${out_dir}/adbauto: ${out_dir}/adbauto.o
	${CXX} ${ldflags} $^ -o $@ ${libcxx_libs} ${BOOST_LIBS}

${out_dir}/smart_adb: ${out_dir}/smart_adb.o
	${CXX} ${ldflags} -o $@ $^ ${libcxx_libs} ${BOOST_LIBS}

