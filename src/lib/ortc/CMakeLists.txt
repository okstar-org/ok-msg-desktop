project(OkRTC)

set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

message(STATUS "${PROJECT_NAME} Implicit include directories: ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}")
message(STATUS "User-added include directories:")
get_directory_property(USER_INCLUDE_DIRECTORIES INCLUDE_DIRECTORIES)
foreach (dir ${USER_INCLUDE_DIRECTORIES})
    message(STATUS "  ${dir}")
endforeach ()

#ok-rtc
find_package(ok-rtc REQUIRED)
if(ok-rtc_FOUND)
  if (WIN32)
      add_definitions(-DWEBRTC_WIN)
  elseif (UNIX)
      add_definitions(-DWEBRTC_POSIX)
  endif ()
  set(webrtc_LIB ok-rtc::ok-rtc)
endif ()

#ok-gloox
find_package(ok-gloox REQUIRED)
if(ok-gloox_FOUND)
    set(gloox_LIB ok-gloox::ok-gloox)
endif ()



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
    webrtc/LogSinkImpl.h
    webrtc/LogSinkImpl.cpp
        ok_rtc_manager.h
        ok_rtc_manager.cpp
        ok_rtc_proxy.h
        ok_rtc_renderer.h
        ok_rtc_defs.h
        ok_rtc_defs.cpp)


add_library(${PROJECT_NAME} STATIC
        ${${PROJECT_NAME}_SOURCES})

target_link_libraries(${PROJECT_NAME}
        PRIVATE ${gloox_LIB}
        PRIVATE ${webrtc_LIB}
)

if (MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif (MSVC)
