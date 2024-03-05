project(OkMSG-Desktop)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(base)
add_subdirectory(lib)
add_subdirectory(modules)
add_subdirectory(UI)

# Add RC files.
set(${PROJECT_NAME}_RC_FILES icon/icon.rc)
# Add QRC files.
set(${PROJECT_NAME}_QRC_FILES UI/resources/resources.qrc)

set(${PROJECT_NAME}_RESOURCES ${${PROJECT_NAME}_RC_FILES}
        ${${PROJECT_NAME}_QRC_FILES})

set(${PROJECT_NAME}_SOURCES
        main.cpp
        application.cpp
        launcher.cpp
)

if (WIN32)
    set(PLATFORM_LIBS
            swresample
            swscale
            avdevice
            avutil
            avcodec
            avfilter
            avformat
            brotlidec-static
            brotlienc-static
            charset
            exif
            fdk-aac
            # freetype
            iconv
            lzma
            mp3lame
            ogg
            openh264
            openjp2
            opus
            postproc
            qrencode
            sqlite3
            vorbis
            vorbisenc
            vorbisfile
            webp
            webpdecoder
            webpdemux
            webpmux
            x264
            x265
            sharpyuv
            OpenAL32
            vpxmt
            bz2
            d3d11
            dxgi
            shlwapi
            Iphlpapi
            dwmapi
            dmoguids
            wmcodecdspuuid
            Winmm
            amstrmid
            msdmo
            strmiids
            psapi
            Vfw32
            shell32
            oleaut32
            ole32
            user32
            advapi32
            bcrypt
            Mf
            Mfplat
            Mfreadwrite
            mfuuid
            dxva2
            evr
            strmbase)
elseif (UNIX)
    set(PLATFORM_LIBS z ssl crypto atomic)
endif ()

set(ALL_LIBRARIES ${PLATFORM_LIBS})

list(APPEND OK_LIBS UI)

if (ENABLE_MODULE_PAINTER)
    list(APPEND OK_LIBS Painter)
endif ()

if (ENABLE_PLUGINS)
    list(APPEND OK_LIBS Plugin)
    include(${CMAKE_SOURCE_DIR}/plugins/plugins.cmake)
endif ()

list(APPEND OK_LIBS
        UI
        IM
        IMCore
        OkRTC
        Session
        Settings
        Network
        Backend
        OkBase
)

if (WIN32 AND NOT LOWER_CMAKE_BUILD_TYPE STREQUAL debug)
    add_executable(${PROJECT_NAME} WIN32 ${${PROJECT_NAME}_SOURCES}
            ${${PROJECT_NAME}_RESOURCES})
else ()
    add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES}
            ${${PROJECT_NAME}_RESOURCES})
endif ()

target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE ${OK_LIBS}
        PRIVATE ${DEP_LIBS}
        PRIVATE ${Qt5Core_LIBRARIES}
        PRIVATE ${Qt5Gui_LIBRARIES}
        PRIVATE ${Qt5Network_LIBRARIES}
        PRIVATE ${Qt5Widgets_LIBRARIES}
        PRIVATE ${Qt5MultimediaWidgets_LIBRARIES}
        PRIVATE ${Qt5Multimedia_LIBRARIES}
        PRIVATE ${Qt5Xml_LIBRARIES}
        PRIVATE ${Qt5Svg_LIBRARIES} ${ALL_LIBRARIES})

if (MSVC)
    set_property(
            TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif (MSVC)

if (LOWER_CMAKE_BUILD_TYPE STREQUAL debug)
    target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG_BUILD)
endif ()
