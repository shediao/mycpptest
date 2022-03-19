#!/bin/bash


if [[ -d "build" ]]; then
  rm -rf ./build/
fi

if [[ ! -d "build" ]]; then
  mkdir build
fi

cd build || exit 1
CC=/opt/llvm/12.x/bin/clang CXX=/opt/llvm/12.x/bin/clang++ \
cmake -G Ninja \
	-DLLVM_TARGETS_TO_BUILD="ARM;AArch64;X86;WebAssembly" \
	-DLLVM_ENABLE_PROJECTS="clang;clang-tools-extra;libcxx;libcxxabi;libunwind;lldb;compiler-rt;lld" \
	-DCMAKE_INSTALL_PREFIX=/opt/llvm/13.x \
	-DCMAKE_BUILD_TYPE=Release \
	../llvm || exit 1
time ninja -C ./ install
