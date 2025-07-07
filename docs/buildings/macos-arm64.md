# Build on macOS(arm64) platform

## 一. 准备环境

- XCode Version 15.0.1 (15A507) (最新版本)
- Homebrew 
- Cmake
- Git
## 二. 安装依赖

  ```bash
  brew install pkg-config make nasm openssl
  brew install Qt@5
  brew install ffmpeg@5 
  brew install mozjpeg libqrencode libexif openh264 openal-soft libx11 range-v3 python3 python-is-python3
  ```   
## 三. 编译安装ok-gloox 

```bash
git clone https://github.com/okstar-org/ok-gloox.git
cd ok-gloox
#如果不使用默认Unix生成器，可添加 -GXCode 生成Xcode项目：cmake -B out -GXcode
cmake -B out
cmake --build out --config Debug
sudo cmake --install out 
  ```
## 四. 编译安装ok-rtc
 
 ```bash
 git clone -b master --recurse-submodules -j8 https://github.com/okstar-org/ok-rtc.git
 cd ok-rtc
 git submodule update --recursive --init 
 #添加ffmpeg路径到PKG_CONFIG_PATH，以让pkg-config正确获取路径
 export PKG_CONFIG_PATH=/opt/homebrew/opt/ffmpeg@5/lib/pkgconfig
 #如果不使用默认Unix生成器，可添加 -GXCode 生成Xcode项目：cmake -B out -GXcode
 cmake -B out 
 cmake --build out --config debug
 sudo cmake --install out
 ``` 
## 五. 编译ok-msg-desktop
```bash
# 配置 PKG_CONFIG_PATH 
export PKG_CONFIG_PATH=/opt/homebrew/opt/ffmpeg@5/lib/pkgconfig:/opt/homebrew/Cellar/openal-soft/1.23.1/lib/pkgconfig
# 添加链接库
export LDFLAGS="-L/opt/homebrew/opt/ffmpeg@5/lib -L/opt/homebrew/Cellar/libexif/0.6.24/lib -L/opt/homebrew/Cellar/qrencode/4.1.1/lib -L/opt/homebrew/Cellar/openal-soft/1.23.1/lib  -latomic -L/opt/homebrew/Cellar/libx11/1.8.10/lib"
#cmake 生成器 unix files
cmake -B out -DCMAKE_PREFIX_PATH=/opt/homebrew/Cellar/qt@5/5.15.13_1 
#cmake 生成器 Xcode，从XcodeIDE打开编译
cmake -B out -DCMAKE_PREFIX_PATH=/opt/homebrew/Cellar/qt@5/5.15.13_1  -GXCode
#build
cmake --build out --config debug
```
### tips
- 由于环境不同，若遇到找不到库，请根据找库类型解决问题。使用PKG_CONFIG_PATH 或 指定CMAKE_PREFIX_PATH
- 链接库问题 添加 LDFLAGS
- 若缺少相关依赖库，请尽量使用brew install
- cmake 配置生成器失败请删除干净目录 rm -rf out


