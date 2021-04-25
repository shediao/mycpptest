# 我的c++测试程序

## 1. 配置基础库环境
### 1.1 boost
```sh
wget https://dl.bintray.com/boostorg/release/1.76.0/source/boost_1_76_0.tar.gz
tar -zxf ./boost_1_76_0.tar.gz
cd ./boost_1_76_0
./bootstrap.sh
./b2 --prefix="/opt/boost/1.76.0" link=static cxxflags="-stdlib=libc++ -std=c++14" linkflags="-stdlib=libc++" install
```
### 1.2 range-v3
```sh
git clone "https://github.com/ericniebler/range-v3.git" $HOME/sources/range-v3
cd $HOME/sources/range-v3 && git checkout -f 0.11.0
```
### 1.3 googletests
```sh
git clone "https://github.com/google/googletest.git" $HOME/sources/googletests
```
### 1.4 llvm/libc++
```sh
git clone "https://github.com/llvm/llvm-project.git" $HOME/sources/llvm-project
mkdir $HOME/sources/llvm-project/out
cd $HOME/sources/llvm-project/out
cmake -DCMAKE_INSTALL_PREFIX="/opt/llvm/libc++" ../
make cxx -j8 VERBOSE=1
make install-cxxabi install-cxx
```
### 1.5 ffmpeg
```sh
git clone "https://git.ffmpeg.org/ffmpeg.git" $HOME/sources/ffmpeg
```

## 2. 编译


