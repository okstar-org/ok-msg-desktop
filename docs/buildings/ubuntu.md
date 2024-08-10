# Ubuntu 构建指南

Version：22.04

## 安装依赖

```shell
sudo apt install -y cmake ninja-build 
sudo apt install -y gcc g++ clang yasm libstdc++-12-dev libc++1
sudo apt install -y qtcreator qtbase5-dev  qtmultimedia5-dev libqt5svg5-dev qttools5-dev qtwebengine5-dev qtwebengine5-dev-tools libqt5websockets5-dev\
sudo apt install -y libcrypto++-dev  libssl-dev
sudo apt install -y libpipewire-0.3-dev libxss-dev libgbm-dev libdrm-dev libxdamage-dev libxrender-dev libxrandr-dev libxtst-dev \
  libasound2-dev libpulse-dev libavcodec-dev libavformat-dev libswscale-dev libavdevice-dev libvpx-dev \
  libopus-dev libjpeg-dev libopenal-dev libopenh264-dev \
  libexif-dev libqrencode-dev libsqlite3-dev
```

## 构建OkRtc库

```shell
git clone https://github.com/okstar-org/ok-rtc.git
cd ok-rtc
# 拉取子模块
git submodule update --init

# CMake 构建
cmake -B out  -DCMAKE_BUILD_TYPE=Release
cmake --build out --config Release
# CMake 安装
sudo cmake --install out --config Release
```

## 编译OkGloox库

```shell
git clone https://github.com/okstar-org/ok-gloox.git
cd ok-gloox
# CMake预处理
cmake -B out -DCMAKE_BUILD_TYPE=Release
# 构建
cmake --build out --config Release
# 执行安装
sudo cmake --install out --config Release
```

## 用Qt Creator 打开OkMSG项目

- 选择最新的QtCreator版本(对CMake的支持更好)。
- 以 ***CMake*** 方式打开项目，即可！