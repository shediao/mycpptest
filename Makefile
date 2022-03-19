
LLVM_ROOT := /opt/llvm/14.x
BOOST_ROOT := /opt/boost/1.78.0
RANGE_V3_ROOT := ${HOME}/sources/range-v3
GTEST_ROOT := ${HOME}/sources/googletest/googletest

CC := ${LLVM_ROOT}/bin/clang
CXX := ${LLVM_ROOT}/bin/clang++

HostOs := $(shell uname -s)

out_dir := out/$(shell uname -s | tr A-Z a-z)-$(shell uname -m)


cflags := -Wall -Werror -Wno-c++11-narrowing -I ${BOOST_ROOT}/include -O0 -g
cxxflags := -I ${RANGE_V3_ROOT}/include -I ${GTEST_ROOT}/include -std=c++17
ldflags := -pthread

ifeq ($(HostOs), Linux)
    ldflags := ${ldflags} -static-libstdc++ -static-libgcc
endif

ldflags := ${ldflags} -L ${BOOST_ROOT}/lib -Wl,-Bstatic -lboost_filesystem -lboost_program_options

.PHONY: all clean run

all: ${out_dir}/process_test ${out_dir}/ranges_test ${out_dir}/filesystem_test ${out_dir}/adb_test ${out_dir}/adbauto ${out_dir}/smart_adb ${out_dir}/program_option_tests
run: all
	@${out_dir}/process_test
	@${out_dir}/filesystem_test
	@${out_dir}/ranges_test
	@${out_dir}/adb_test
	@${out_dir}/program_option_test

${out_dir}:
	mkdir -p ${out_dir}

${out_dir}/%.o: %.cc | ${out_dir} Makefile
	${CXX} ${cflags} ${cxxflags} -I ${GTEST_ROOT} -c $< -o $@
${out_dir}/%.o : ${GTEST_ROOT}/src/%.cc | ${out_dir} Makefile
	${CXX} ${cflags} ${cxxflags} -I ${GTEST_ROOT} -c $< -o $@

clean:
	rm -rf ${out_dir}

${out_dir}/gtest-all.o : ${GTEST_ROOT}/src/gtest-all.cc | ${out_dir}
	${CXX} ${cflags} ${cxxflags} -I ${GTEST_ROOT} -c $< -o $@
${out_dir}/gtest_main.o : ${GTEST_ROOT}/src/gtest_main.cc | ${out_dir}
	${CXX} ${cflags} ${cxxflags} -I ${GTEST_ROOT} -c $< -o $@

${out_dir}/process_test: ${out_dir}/gtest-all.o ${out_dir}/process_test.o
	${CXX} $^ -o $@ ${ldflags}

${out_dir}/ranges_test: ${out_dir}/ranges_test.o ${out_dir}/gtest-all.o
	${CXX} $< -o $@ ${ldflags}

${out_dir}/filesystem_test: ${out_dir}/filesystem_test.o ${out_dir}/gtest-all.o ${out_dir}/gtest_main.o
	${CXX} $^ -o $@ ${ldflags}

${out_dir}/adb_test: ${out_dir}/gtest-all.o ${out_dir}/adb_test.o ${out_dir}/adb.o
	${CXX} $^ -o $@ ${ldflags}

${out_dir}/adbauto: ${out_dir}/adbauto.o
	${CXX} $^ -o $@ ${ldflags}

${out_dir}/smart_adb: ${out_dir}/smart_adb.o
	${CXX} -o $@ $^ ${ldflags}

${out_dir}/program_option_tests: ${out_dir}/program_option_tests.o
	${CXX} -o $@ $^ ${ldflags}

