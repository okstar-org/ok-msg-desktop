project(OkRTC)

message(STATUS "Implicit include directories: ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}")
message(STATUS "User-added include directories:")
get_directory_property(USER_INCLUDE_DIRECTORIES INCLUDE_DIRECTORIES)
foreach (dir ${USER_INCLUDE_DIRECTORIES})
    message(STATUS "  ${dir}")
endforeach ()

#WebRTC
if(${DESKTOP_APP_USE_FLATPAK})
    include_directories(${webrtc_SOURCE_DIR})
    include_directories(${webrtc_SOURCE_DIR}/third_party/sqlite)
    include_directories(${webrtc_SOURCE_DIR}/third_party/abseil-cpp)
    message(STATUS "webrtc_SOURCE_DIR=${webrtc_SOURCE_DIR}")
    message(STATUS "webrtc_LIB=${webrtc_LIB}")
else ()
    FetchContent_Declare(protobuf
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
        GIT_TAG        v26.1
    )
    FetchContent_MakeAvailable(protobuf)
    include_directories(${protobuf_SOURCE_DIR})

    message(STATUS "protobuf_SOURCE_DIR=${protobuf_SOURCE_DIR}")
    message(STATUS "protobuf_LIB=${protobuf_LIB}")


    FetchContent_Declare(webrtc
        GIT_REPOSITORY https://github.com/okstar-org/ok-rtc.git
        GIT_TAG        master
    )
    FetchContent_MakeAvailable(webrtc)
    include_directories(${webrtc_SOURCE_DIR}/src)
    set(webrtc_LIB ok-rtc)
    message(STATUS "webrtc_SOURCE_DIR=${webrtc_SOURCE_DIR}")
    message(STATUS "webrtc_LIB=${webrtc_LIB}")
endif ()

add_definitions(-DOPENSSL_IS_BORINGSSL=1)
add_definitions(-DNDEBUG)

if (WIN32)
    add_definitions(-DWEBRTC_WIN)
elseif (UNIX)
    add_definitions(-DWEBRTC_POSIX)
endif ()

include_directories(${gloox_SOURCE_DIR}/src)


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
        ok_rtc_manager.h
        ok_rtc_manager.cpp
        ok_rtc_proxy.h
        ok_rtc_renderer.h
        ok_rtc_defs.h
        ok_rtc_defs.cpp)

include_directories(${CMAKE_SOURCE_DIR}/3rdparty)

add_library(${PROJECT_NAME} STATIC
        ${${PROJECT_NAME}_SOURCES})

target_link_libraries(${PROJECT_NAME}
        PRIVATE ${gloox_LIB}
        PRIVATE ${webrtc_LIB}
)

if (MSVC)
    set_property(
            TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif (MSVC)
