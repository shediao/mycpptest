

CC := clang-11
CXX := clang++-11

cflags := -Wall -Werror -I /opt/boost/1.75.0/include -g2 -O0
cxxflags := -std=c++17

ldflags := -O3 -Wl,-Bdynamic \
    -L /opt/boost/1.75.0/lib -lboost_filesystem \
    -lboost_atomic \
    -lboost_chrono \
    -lboost_container \
    -lboost_context \
    -lboost_contract \
    -lboost_coroutine \
    -lboost_date_time \
    -lboost_exception \
    -lboost_fiber \
    -lboost_graph \
    -lboost_iostreams \
    -lboost_json \
    -lboost_locale \
    -lboost_log \
    -lboost_log_setup \
    -lboost_nowide \
    -lboost_program_options \
    -lboost_random \
    -lboost_regex \
    -lboost_serialization \
    -lboost_stacktrace_addr2line \
    -lboost_stacktrace_backtrace \
    -lboost_stacktrace_basic \
    -lboost_stacktrace_noop \
    -lboost_system \
    -lboost_thread \
    -lboost_timer \
    -lboost_type_erasure \
    -lboost_wserialization -Wl,-rpath=/opt/boost/1.75.0/lib \
    -Wl,-Bdynamic


.PHONY: all clean run

run: process_test
	./process_test

all: process_test

clean:
	find . -name '*.o' -delete

process_test.o: process_test.cc
	${CXX} ${cflags} ${cxxflags} -c $< -o $@

process_test: process_test.o
	${CXX} ${ldflags} $< -o $@
