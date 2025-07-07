# Build on macOS(x64) platform

- 安装 XCode 等相关c/c++ clang开发环境，相关方法略。
- 安装 Qt5 开发环境
    - 访问：https://mirrors.tuna.tsinghua.edu.cn/qt/official_releases/online_installers/
    - 下载qt-unified-mac-x64-online.dmg 安装
    - 运行请输入自己帐号
    - 在组件选择列表，可能没有qt5的相关列表，请勾选右侧"Archived或归档"按钮再"Filter或筛选"即可加载5.15版本相关包。
    - 选择：
        - Qt: Qt5.15.2、QtWebEngine
        - Developer tools: Qt Creator、CMake、Ninja
    - 之后就是下一步、安装即可。

## Install brew

> If you have not brew, please install it.

```shell
# install command is    
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```

## Install dependencies

```shell
brew install pkg-config make yasm openssl@1.1
brew install mozjpeg libqrencode libexif openh264 openal-soft ffmpeg@5 range-v3 python3 python-is-python3
```

## Build ok-rtc library

```shell
git clone -b master https://github.com/okstar-org/ok-rtc.git
cd ok-rtc

# 拉取子模块
git submodule update --init

# 设置pkg-config路径
export PKG_CONFIG_PATH="/usr/local/opt/openssl@1.1/lib/pkgconfig:/usr/local/opt/mozjpeg/lib/pkgconfig:/usr/local/opt/ffmpeg@5/lib/pkgconfig"

# 预处理
cmake -B out -DCMAKE_BUILD_TYPE=Debug \
-DJPEG_LIBRARY=/usr/local/opt/mozjpeg/lib \
-DJPEF_INCLUDE_DIR=/usr/local/opt/mozjpeg/include

# - 如果出现找不到 src/third_party/crc32等以及其它第三方包缺少问题，请删除 src/third_party/crc32目录
# - 重新执行`git submodule update --init`拉取依赖库，可能是由于上一次未下载完成

# 构建
cmake --build out --config=Debug

```

## Build ok-gloox library

```shell
git clone https://github.com/okstar-org/ok-gloox.git
cd ok-gloox
# CMake预处理
cmake -B out -DCMAKE_BUILD_TYPE=Release
# 构建
cmake --build out --config Release
# 执行安装
cmake --install out --config Release
```

## 用Qt Creator 打开OkMSG项目

- 选择最新的QtCreator版本(对CMake的支持更好)。
- 以 ***CMake*** 方式打开项目，即可！

