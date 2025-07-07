# Ubuntu 构建指南

Version：Ubuntu 24.04+

## 安装依赖

```shell
sudo apt install -y cmake ninja-build 
sudo apt install -y gcc g++ clang yasm libstdc++-12-dev libc++1
sudo apt install -y qtcreator qtbase5-dev  qtmultimedia5-dev libqt5svg5-dev qttools5-dev qtwebengine5-dev qtwebengine5-dev-tools libqt5websockets5-dev
sudo apt install -y libcrypto++-dev  libssl-dev
sudo apt install -y libpipewire-0.3-dev libxss-dev libgbm-dev libdrm-dev libxdamage-dev libxrender-dev libxrandr-dev libxtst-dev \
  libasound2-dev libpulse-dev libavcodec-dev libavformat-dev libswscale-dev libavdevice-dev libvpx-dev \
  libopus-dev libjpeg-dev libopenal-dev libopenh264-dev \
  libexif-dev libqrencode-dev libsqlite3-dev librange-v3-dev \
  python3 python-is-python3
```

## 构建OkRtc库

```shell
git clone -b master https://github.com/okstar-org/ok-rtc.git
cd ok-rtc
# 拉取子模块
git submodule update --init

# CMake 构建(Debug)
cmake -B out-Debug -DCMAKE_BUILD_TYPE=Debug
cmake --build out-Debug --config Debug
sudo cmake --install out-Debug --config Debug # 管理员执行

# CMake 构建(Release)
cmake -B out-Release -DCMAKE_BUILD_TYPE=Release
cmake --build out-Release --config Release
sudo cmake --install out-Release --config Release # 管理员执行

# CMake 构建(RelWithDebInfo)
cmake -B out-RelWithDebInfo -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build out-RelWithDebInfo --config RelWithDebInfo
sudo cmake --install out-RelWithDebInfo --config RelWithDebInfo # 管理员执行

# CMake 构建(MinSizeRel)
cmake -B out-MinSizeRel -DCMAKE_BUILD_TYPE=MinSizeRel
cmake --build out-MinSizeRel --config MinSizeRel
sudo cmake --install out-MinSizeRel --config MinSizeRel # 管理员执行
```

## 编译OkGloox库

```shell
git clone https://github.com/okstar-org/ok-gloox.git
cd ok-gloox

# CMake 构建(Debug)
cmake -B out-Debug -DCMAKE_BUILD_TYPE=Debug
cmake --build out-Debug --config Debug
sudo cmake --install out-Debug --config Debug # 管理员执行

# CMake 构建(Release)
cmake -B out-Release -DCMAKE_BUILD_TYPE=Release
cmake --build out-Release --config Release
sudo cmake --install out-Release --config Release # 管理员执行
```

## 用Qt Creator 打开OkMSG项目

- 选择最新的QtCreator版本(对CMake的支持更好)。
- 以 ***CMake*** 方式打开项目，即可！


## OkMSG项目编译失败解决方案

# 编译提示错误:  /Qt/5.15.2/gcc_64/lib/libQt5WebEngineCore.so.5.15.2: .dynsym local symbol at index 3 (>= sh_info of 3)
解决方案：
在src/CMakeList.txt中添加命令：
if (LINUX)
    set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=gold")
endif()

详情可看: https://bugreports.qt.io/browse/QTBUG-80964



