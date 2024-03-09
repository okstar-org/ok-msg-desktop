project(OkRTC)
message(STATUS "Implicit include directories: ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}")
message(STATUS "User-added include directories:")
get_directory_property(USER_INCLUDE_DIRECTORIES INCLUDE_DIRECTORIES)
foreach(dir ${USER_INCLUDE_DIRECTORIES})
    message(STATUS "  ${dir}")
endforeach()

#WebRTC
set(WebRTC_VER "121.6167.5.0")
if (WIN32)
    set(WebRTC_URL "https://github.com/crow-misia/libwebrtc-bin/releases/download/${WebRTC_VER}/libwebrtc-win-x64.7z")
elseif (LINUX)
    set(WebRTC_URL "https://github.com/crow-misia/libwebrtc-bin/releases/download/${WebRTC_VER}/libwebrtc-linux-x64.tar.xz")
endif ()

message(STATUS "Fetch webrtc")
FetchContent_Declare(
        webrtc
        URL ${WebRTC_URL}
)
FetchContent_MakeAvailable(webrtc)
include_directories(${webrtc_SOURCE_DIR}/include)
include_directories(${webrtc_SOURCE_DIR}/include/third_party/abseil-cpp/)




add_definitions(-DOPENSSL_IS_BORINGSSL=1)

if (WIN32)
    add_definitions(-DWEBRTC_WIN)
endif ()

if (UNIX)
    add_definitions(-DWEBRTC_POSIX)
endif ()

add_definitions(-DNDEBUG)

set(${PROJECT_NAME}_SOURCES
        webrtc/ok_videosink.h
        webrtc/ok_videosink.cpp
        webrtc/ok_rtc.h
        webrtc/ok_rtc.cpp
        webrtc/ok_conductor.h
        webrtc/ok_conductor.cc
        webrtc/test_video_capturer.h
        webrtc/test_video_capturer.cc
        webrtc/vcm_capturer.h
        webrtc/vcm_capturer.cc

        ok_rtc_manager.h ok_rtc_manager.cpp ok_rtc_proxy.h
        ok_rtc_renderer.h ok_rtc_defs.h ok_rtc_defs.cpp)

include_directories(${CMAKE_SOURCE_DIR}/3rdparty)

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})


target_link_libraries(${PROJECT_NAME}
        ${webrtc_SOURCE_DIR}/lib/libwebrtc.a

        X11
)

if (MSVC)
    set_property(
            TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif (MSVC)
