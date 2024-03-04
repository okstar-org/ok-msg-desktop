project(OkRTC)

add_definitions(-DOPENSSL_IS_BORINGSSL=1)

set(PROJECT_Sources
        ok_rtc_manager.h
        ok_rtc_manager.cpp
        ok_rtc_proxy.h
        ok_rtc_renderer.h
        ok_rtc_defs.h
        ok_rtc_defs.cpp
        )

add_subdirectory(webrtc)

include_directories(${CMAKE_SOURCE_DIR}/3rdparty)
include_directories(${GLOOX_DIR}/include)

include_directories(${WebRTC_DIR}/include)
include_directories(${WebRTC_DIR}/include/third_party)
include_directories(${WebRTC_DIR}/include/third_party/abseil-cpp)
include_directories(${WebRTC_DIR}/include/third_party/libyuv)
include_directories(${WebRTC_DIR}/include/third_party/libyuv/include)

if (WIN32)
    set(DEP_LIBS
            ${WebRTC_DIR}/${LOWER_CMAKE_BUILD_TYPE}/webrtc.lib
            )

elseif (UNIX)
    set(DEP_LIBS
            ${WebRTC_DIR}/${LOWER_CMAKE_BUILD_TYPE}/libwebrtc.a
            gloox
            )
            
endif ()

add_library(${PROJECT_NAME} ${PROJECT_Sources})

if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif (MSVC)