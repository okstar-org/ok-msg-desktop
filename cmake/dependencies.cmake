# 设置Qt配置参数，默认从环境变量读取
if(WIN32)
    set(QT_DIR $ENV{QTDIR})
    if(NOT DEFINED QT_DIR)
        message(FATAL_ERROR "请在环境变量配置Qt路径【QTDIR】！")
    endif()
    message(STATUS "QT_DIR=${QT_DIR}")
    set(CMAKE_PREFIX_PATH ${QT_DIR})
    set(CMAKE_BUILD_TYPE "Release")

    # 根据Qt类型，设置动态(安装默认)或者静态(下载的静态版)，默认从环境变量读取
    set(LINK_STATIC_QT $ENV{LINK_STATIC_QT})
    if(NOT DEFINED LINK_STATIC_QT)
        message(FATAL_ERROR "请在环境变量配置Qt类型【LINK_STATIC_QT】！")
    endif()
    message(STATUS "LINK_STATIC_QT=${LINK_STATIC_QT}")
endif()

# webrtc
include(FetchContent)
set(WebRTC_VER "121.6167.5.0")

if(WIN32)
set(WebRTC_URL "https://github.com/crow-misia/libwebrtc-bin/releases/download/${WebRTC_VER}/libwebrtc-win-${ARCH}.7z")
else()
set(WebRTC_URL "https://github.com/crow-misia/libwebrtc-bin/releases/download/${WebRTC_VER}/libwebrtc-${PLATFORM}-${ARCH}.tar.xz")
endif()

message(STATUS "Fetch webrtc")
FetchContent_Declare(
        webrtc
        URL ${WebRTC_URL}
)
FetchContent_MakeAvailable(webrtc)

# absl
message(STATUS "Fetch absl")
set(ABSL_PROPAGATE_CXX_STD ON)
FetchContent_Declare(absl
      GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
      GIT_TAG lts_2023_08_02
      #  URL https://github.com/abseil/abseil-cpp/releases/download/20220623.2/abseil-cpp-20220623.2.tar.gz
)
FetchContent_MakeAvailable(absl)
