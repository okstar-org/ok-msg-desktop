project(OkRTC)

#WebRTC
include(FetchContent)
FetchContent_Declare(
        webrtc
#     121.6167.5.0   114.5735.6.1
        URL https://github.com/crow-misia/libwebrtc-bin/releases/download/121.6167.5.0/libwebrtc-linux-x64.tar.xz
)
FetchContent_MakeAvailable(webrtc)
include_directories(${webrtc_SOURCE_DIR}/include)


FetchContent_Declare(
        absl
        GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
)
FetchContent_MakeAvailable(absl)
include_directories(${absl_SOURCE_DIR})

add_definitions(-DOPENSSL_IS_BORINGSSL=1)

if(WIN32)
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
        gloox
        X11
)

if (MSVC)
    set_property(
            TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif (MSVC)
