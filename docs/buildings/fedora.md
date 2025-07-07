# Fedora

- Fedora 39

## 安装依赖

```shell
dnf update -y
dnf install -y gcc g++
dnf install -y qt5-qtbase-devel qt6-qtbase-gui  qt5-qtmultimedia-devel \
  qt5-qtsvg-devel qt5-qttools-devel qt5-qttools-static \
  libavcodec-free-devel libavdevice-free-devel \
  libexif-free-devel qrencode-devel sqlite3-devel \
  libvpx-devel openal-soft-devel openssl-devel  \
  python3 python-is-python3
```

### 构建OkRtc模块

```shell
git clone -b master https://github.com/okstar-org/ok-rtc.git
cd ok-rtc
# 拉取子模块
git submodule update --init

# configuration
cmake -B out -DCMAKE_BUILD_TYPE=Debug
# build
cmake --build out --config=Debug
# install
sudo cmake --install out --config=Debug
```

### 编译OkGloox库

```shell
git clone https://github.com/okstar-org/ok-gloox.git
cd ok-gloox
# configuration
cmake -B out -DCMAKE_BUILD_TYPE=Debug
# build
cmake --build out --config=Debug
# install
sudo cmake --install out --config=Debug
```

### 构建项目

```shell
# configuration
# [-DOK_CPACK=1  #(打包DEB、RPM)]
cmake -B build -DCMAKE_BUILD_TYPE={Debug|Release} 

# 构建
cmake --build build [--target package #(打包DEB、RPM)]
```
