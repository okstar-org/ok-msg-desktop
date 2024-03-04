PROJECT(OkRTC_webrtc)

include_directories(${WebRTC_DIR}/include)
include_directories(${WebRTC_DIR}/include/third_party)
include_directories(${WebRTC_DIR}/include/third_party/abseil-cpp)
include_directories(${WebRTC_DIR}/include/third_party/libyuv)
include_directories(${WebRTC_DIR}/include/third_party/libyuv/include)
include_directories(${GLOOX_DIR}/include)

set(${PROJECT_NAME}_SOURCES
        ok_videosink.h
        ok_videosink.cpp
        ok_rtc.h
        ok_rtc.cpp
        ok_conductor.h
        ok_conductor.cc
        test_video_capturer.h
        test_video_capturer.cc
        vcm_capturer.h
        vcm_capturer.cc
        )

add_library(${PROJECT_NAME}  ${${PROJECT_NAME}_SOURCES})

if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif (MSVC)