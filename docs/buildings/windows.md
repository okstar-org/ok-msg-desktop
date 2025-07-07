# Windows 平台构建

## 安装必备依赖

- 安装`visual studio 17 2022`
  - 选择 `Windows SdK 10 20348`
- 安装`strawberry-perl`
- 安装`OpenSSL 1.1.1`且版本的一致
- 安装`Python3`和`openpyxl`插件
```shell
  python -m pip install --upgrade pip
  pip install openpyxl
```

> 可以使用Qt自带的OpenSSL的也可以自行安装，配置bin目录到环境变量到PATH！

### 配置vcpkg

> 以为VS已经自带vcpkg，所以无需安装

```shell
#设置vcpkg路径，也可以参考官网下载：https://github.com/microsoft/vcpkg/blob/master/README_zh_CN.md
VCPKG_ROOT="E:\Program Files\Microsoft Visual Studio\2022\Community\VC\vcpkg"
#可选，默默C盘
VCPKG_DOWNLOADS=下载路径
```

- 安装vcpkg依赖包

```shell
# 进入项目跟目录（包含vcpkg.json），执行安装命令
vcpkg install --triplet x64-windows
```

- 配置pkg-config关联
  这一步实现pkg-config到vcpkg安装包的关联，便于cmake pkg-config模块能检索到。

```shell
# 配置环境变量
PKG_CONFIG_PATH=<项目根目录>/vcpkg_installed/x64-windows/lib/pkgconfig
```

命令行输入如下，检查是否存在vcpkg安装的新包。

    pkg-config.bat --list-all

## 构建OkRTC库

```shell
git clone -b master https://github.com/okstar-org/ok-rtc.git
cd ok-rtc
# 拉取子模块
git submodule update --init

# CMake 构建(Debug)
cmake -B out-Debug -A x64 -DCMAKE_BUILD_TYPE=Debug
cmake --build out-Debug --config Debug
cmake --install out-Debug --config Debug # 管理员执行

# CMake 构建(Release)
cmake -B out-Release -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build out-Release --config Release
cmake --install out-Release --config Release # 管理员执行

# CMake 构建(RelWithDebInfo)
cmake -B out-RelWithDebInfo -A x64 -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build .\out-RelWithDebInfo\ --config RelWithDebInfo
cmake --install out-RelWithDebInfo --config RelWithDebInfo # 管理员执行

# CMake 构建(MinSizeRel)
cmake -B out-MinSizeRel -A x64 -DCMAKE_BUILD_TYPE=MinSizeRel
cmake --build .\out-MinSizeRel\ --config MinSizeRel
cmake --install out-MinSizeRel --config MinSizeRel # 管理员执行
```

## 构建OkGloox库

```shell
git clone https://github.com/okstar-org/ok-gloox.git
cd ok-gloox

# CMake 构建(Debug)
cmake -B out-Debug -A x64 -DCMAKE_BUILD_TYPE=Debug
cmake --build out-Debug --config Debug
cmake --install out-Debug --config Debug # 管理员执行

# CMake 构建(Release)
cmake -B out-Release -A x64 -DCMAKE_BUILD_TYPE=Release
cmake --build out-Release --config Release
cmake --install out-Release --config Release # 管理员执行

# CMake 构建(RelWithDebInfo)
cmake -B out-RelWithDebInfo -A x64 -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build .\out-RelWithDebInfo\ --config RelWithDebInfo
cmake --install out-RelWithDebInfo --config RelWithDebInfo # 管理员执行

# CMake 构建(MinSizeRel)
cmake -B out-MinSizeRel -A x64 -DCMAKE_BUILD_TYPE=MinSizeRel
cmake --build .\out-MinSizeRel\ --config MinSizeRel
cmake --install out-MinSizeRel --config MinSizeRel # 管理员执行
```

## 构建OkMSG项目

- 执行构建命令

```shell
# 预处理
cmake -B out --preset win-x64-debug #或者选择win-x64-release
cmake --build out --config Debug #或者Release
```

- 增加构建环境（该步骤为可选）

> - CMakePresets.json 为平台无关性公共全局配置（请勿修改）。
> - CMakeUserPresets.json 该文件是针对用户本地环境的配置（不要提交)。
> - 利用 `CMAKE_PREFIX_PATH` 关联到第三方库（调试库），比如：Qt、VcPkg下载的库、OkRTC等，再比如要增加gcc构建环境。

修改CMake预设文件CMakeUserPresets.json(该文件是针对用户本地环境的配置，不要提交)，列子如下：

```json
{
  "version": 3,
  "configurePresets": [
    {
      "name": "win-x64-release",
      "displayName": "Windows x64 Release",
      "binaryDir": "${sourceDir}/out/${presetName}",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Release",
        "CMAKE_PREFIX_PATH": "E:/QtWorkspace/ok-rtc/out/Release;${sourceDir}/vcpkg_installed/x64-windows;E:/Qt/Qt5.15.7-Windows-x86_64-VS2019-16.11.20-staticFull"
      }
    },
    {
      "name": "win-x64-debug",
      "displayName": "Windows x64 Debug",
      "binaryDir": "${sourceDir}/out/${presetName}",
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CMAKE_PREFIX_PATH": "E:/QtWorkspace/ok-rtc/out/Debug;${sourceDir}/vcpkg_installed/x64-windows;E:/Qt/Qt5.15.11-Windows-x86_64-VS2022-staticFull-debug"
      }
    },
    {
      "name": "gcc",
      "displayName": "GCC 11.4.0 x86_64-linux-gnu",
      "description": "使用编译器: C = /usr/bin/gcc, CXX = /usr/bin/g++",
      "binaryDir": "${sourceDir}/out/build/${presetName}",
      "cacheVariables": {
        "CMAKE_INSTALL_PREFIX": "${sourceDir}/out/install/${presetName}",
        "CMAKE_C_COMPILER": "/usr/bin/gcc",
        "CMAKE_CXX_COMPILER": "/usr/bin/g++",
        "CMAKE_BUILD_TYPE": "Debug"
      }
    }
  ]
}
```

## 用Qt Creator 打开OkMSG项目

- 选择最新的QtCreator版本(对CMake的支持更好)。
- 以 ***CMake*** 方式打开项目，即可！

> 首次打开需要加载vcpkg以及下载和构建相关依赖，需要耗费一些时间，请耐心等待！
