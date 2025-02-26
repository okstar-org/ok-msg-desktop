project(OkRTC)

#ok-rtc
find_package(ok-rtc REQUIRED)
message(STATUS "Found ok-rtc=${ok-rtc_FOUND}")

if (ok-rtc_FOUND)
    if (WIN32)
        add_definitions(-DWEBRTC_WIN=1)
    elseif (UNIX)
        add_definitions(-DWEBRTC_POSIX=1 -DWEBRTC_LINUX=1 -DABSL_USES_STD_STRING_VIEW=1)
    endif ()
    set(webrtc_LIB ok-rtc::ok-rtc)
endif ()

#ok-gloox
find_package(ok-gloox REQUIRED)
message(STATUS "Found ok-gloox=${ok-gloox_FOUND}")
if (ok-gloox_FOUND)
    set(gloox_LIB ok-gloox)
endif ()

pkg_check_modules(JPEG REQUIRED libjpeg)
include_directories(${JPEG_INCLUDE_DIRS})
list(APPEND ALL_LIBRARIES ${JPEG_LINK_LIBRARIES})

pkg_check_modules(exif REQUIRED libexif)
include_directories(${exif_INCLUDE_DIRS})
list(APPEND ALL_LIBRARIES ${exif_LINK_LIBRARIES})

message(STATUS "${PROJECT_NAME} Implicit include directories: ${CMAKE_CXX_IMPLICIT_INCLUDE_DIRECTORIES}")
message(STATUS "User-added include directories:")
get_directory_property(USER_INCLUDE_DIRECTORIES INCLUDE_DIRECTORIES)
foreach (dir ${USER_INCLUDE_DIRECTORIES})
    message(STATUS "  ${dir}")
endforeach ()

set(${PROJECT_NAME}_SOURCES
    webrtc/platform/PlatformInterface.h
    # Desktop capturer
    webrtc/platform/desktop_capturer/DesktopCaptureSource.h
    webrtc/platform/desktop_capturer/DesktopCaptureSource.cpp
    webrtc/platform/desktop_capturer/DesktopCaptureSourceHelper.h
    webrtc/platform/desktop_capturer/DesktopCaptureSourceHelper.cpp
    webrtc/platform/desktop_capturer/DesktopCaptureSourceManager.h
    webrtc/platform/desktop_capturer/DesktopCaptureSourceManager.cpp

    webrtc/platform/desktop/DesktopInterface.cpp
    webrtc/platform/desktop/DesktopInterface.h
    webrtc/platform/desktop/VideoCapturerInterfaceImpl.cpp
    webrtc/platform/desktop/VideoCapturerInterfaceImpl.h
    webrtc/platform/desktop/VideoCapturerTrackSource.cpp
    webrtc/platform/desktop/VideoCapturerTrackSource.h
    webrtc/platform/desktop/VideoCameraCapturer.cpp
    webrtc/platform/desktop/VideoCameraCapturer.h

    webrtc/Stats.h
    webrtc/Instance.h
    webrtc/VideoCaptureInterface.h
    webrtc/VideoCaptureInterface.cpp
    webrtc/VideoCaptureInterfaceImpl.h
    webrtc/VideoCaptureInterfaceImpl.cpp
    webrtc/StaticThreads.h
    webrtc/StaticThreads.cpp
    webrtc/ok_video_sink.h
    webrtc/ok_video_sink.cpp
    webrtc/webrtc.h
    webrtc/webrtc.cpp
    webrtc/ok_conductor.h
    webrtc/ok_conductor.cpp
    webrtc/LogSinkImpl.h
    webrtc/LogSinkImpl.cpp
    ok_rtc.h
    ok_rtc.cpp
    ok_rtc_manager.h
    ok_rtc_manager.cpp
    utils/image_convert.h
    utils/image_convert.cpp
    )


add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES})


if (WIN32)
    set(PLATFORM_LIBS   # 3D
        d3d11
        # GDI图形API
        gdi32
        # DirectX
        dxgi dxva2
        # Windows Media
        Winmm dmoGuids WMCodecDspUuid avrt
        # Media Foundation API
        Mf Mfplat Mfreadwrite mfuuid evr #Enhanced Video Renderer 视频渲染器
        # DirectShow
        amstrmid msdmo strmiids strmbase
        # COM
        Ole32 OleAut32 uuid OLDNAMES comsuppd
        # Windows DWM API
        dwmapi Shcore
        # Shell Light-Weight Utility API
        shlwapi
        # IP Helper API
        iphlpapi
        # 核心API
        kernel32 user32 Vfw32 shell32 advapi32
        )

    if (CMAKE_BUILD_TYPE MATCHES "Debug")
        set(PLATFORM_LIBS ${PLATFORM_LIBS} MSVCRTD)
    else ()
        set(PLATFORM_LIBS ${PLATFORM_LIBS} MSVCRT)
    endif ()

elseif (APPLE)

    find_library(APPLICATION_SERVICES ApplicationServices)
    find_library(CORE_FOUNDATION CoreFoundation)
    find_library(CORE_SERVICES CoreServices)
    find_library(COREGRAPHICS CoreGraphics)
    find_library(CORE_AUDIO CoreAudio)
    find_library(CORE_VIDEO CoreVIDEO)
    find_library(CoreMedia CoreMedia)
    find_library(AudioToolbox AudioToolbox)
    find_library(VideoToolbox VideoToolbox)
    find_library(OpenGL OpenGL)
    find_library(IOKit IOKit)
    find_library(AppKit AppKit)
    find_library(IOSurface IOSurface)
    find_library(AVFOUNDATION AVFoundation)
    find_library(Foundation Foundation)
    find_library(Metal Metal)

    set(PLATFORM_LIBS ${AppKit} ${Metal} ${IOSurface} ${AudioToolbox} ${VideoToolbox}
        ${AVFOUNDATION} ${Foundation} ${IOKit} ${OpenGL}
        ${CoreMedia} ${CORE_AUDIO} ${CORE_VIDEO} ${COREGRAPHICS}
        ${CORE_FOUNDATION} ${APPLICATION_SERVICES} ${CORE_SERVICES})

elseif (LINUX)
    set(PLATFORM_LIBS z atomic X11)
endif ()

target_link_libraries(${PROJECT_NAME}
    PRIVATE ${webrtc_LIB}
    PRIVATE ${gloox_LIB}
    PUBLIC ${ALL_LIBRARIES}
    PUBLIC ${PLATFORM_LIBS}
    PRIVATE range-v3::meta range-v3::concepts range-v3::range-v3
    )

if (WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()


# Google Test
if(LINUX)
    FILE(GLOB ${PROJECT_NAME}_TEST_RESOURCES test/*.cpp)
    message(STATUS "Test files: ${${PROJECT_NAME}_TEST_RESOURCES}")

    add_executable(${PROJECT_NAME}Tests
        ${${PROJECT_NAME}_TEST_RESOURCES}
        ${${PROJECT_NAME}_SOURCES})

    # 链接gtest和gtest_main库
    target_link_libraries(${PROJECT_NAME}Tests
        PRIVATE gtest_main
        PRIVATE ${webrtc_LIB}
        PRIVATE ${gloox_LIB}
        PRIVATE ${Qt5Core_LIBRARIES}
        PRIVATE ${Qt5Gui_LIBRARIES}
        PRIVATE ${Qt5Svg_LIBRARIES}
        PRIVATE ${Qt5Xml_LIBRARIES}
        PUBLIC ${ALL_LIBRARIES}
        PUBLIC ${PLATFORM_LIBS}
        PRIVATE range-v3::meta range-v3::concepts range-v3::range-v3)

    # 添加测试到CTest（如果你使用CTest）
    add_test(NAME Run${PROJECT_NAME}Tests COMMAND ${PROJECT_NAME}Tests)
endif()

