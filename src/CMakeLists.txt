project(ok-msg-desktop)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(base)
add_subdirectory(lib)
add_subdirectory(modules)
add_subdirectory(UI)

# Add RC files.
set(${PROJECT_NAME}_RC_FILES
    icon/icon.rc)

# Add QRC files.
set(${PROJECT_NAME}_QRC_FILES
    resources/resources.qrc)

set(${PROJECT_NAME}_RESOURCES
    ${${PROJECT_NAME}_RC_FILES}
    ${${PROJECT_NAME}_QRC_FILES})

set(${PROJECT_NAME}_SOURCES
    main.cpp
    application.h
    application.cpp
    launcher.h
    launcher.cpp
    ipc.h
    ipc.cpp
    Bus.h
    Bus.cpp
)

list(APPEND OK_LIBS
    UI
    Platform
    IM
    OkSession
    OkLog
    OkSettings
    OkNetwork
    OkBackend
    OkBase
)

if(ENABLE_PLUGINS)
    list(APPEND OK_LIBS Plugin)
endif()

if (WIN32 AND CMAKE_BUILD_TYPE STREQUAL Release)
    add_executable(${PROJECT_NAME} WIN32
        ${${PROJECT_NAME}_SOURCES}
        ${${PROJECT_NAME}_RESOURCES})
else ()
    add_executable(${PROJECT_NAME}
        ${${PROJECT_NAME}_SOURCES}
        ${${PROJECT_NAME}_RESOURCES})
endif ()

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC ${OK_LIBS}
)

if (WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>") 
endif ()

set(PNG_NAME "ok-msg.png")

if (LINUX AND DESKTOP_APP_USE_PACKAGED)
    message(STATUS "CMAKE_INSTALL_DATAROOTDIR=${CMAKE_INSTALL_DATAROOTDIR}")
    message(STATUS "CMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_BINDIR}")

    include(GNUInstallDirs)
    configure_file("../lib/xdg/${APPLICATION_EXE_NAME}.metainfo.xml.in" "${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_EXE_NAME}.metainfo.xml" @ONLY)
    generate_appdata_changelog(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/changelog.txt" "${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_EXE_NAME}.metainfo.xml")
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" BUNDLE DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_EXE_NAME}.metainfo.xml" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/metainfo" RENAME "${DESKTOP_LAUNCHER_BASENAME}.metainfo.xml")
    install(FILES "../lib/xdg/${APPLICATION_EXE_NAME}.desktop" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications" RENAME "${DESKTOP_LAUNCHER_BASENAME}.desktop")
    install(FILES "../src/resources/logo/icon-16.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/16x16/apps" RENAME ${PNG_NAME})
    install(FILES "../src/resources/logo/icon-32.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/32x32/apps" RENAME ${PNG_NAME})
    install(FILES "../src/resources/logo/icon-48.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/48x48/apps" RENAME ${PNG_NAME})
    install(FILES "../src/resources/logo/icon-64.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/64x64/apps" RENAME ${PNG_NAME})
    install(FILES "../src/resources/logo/icon-128.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps" RENAME ${PNG_NAME})
    install(FILES "../src/resources/logo/icon-256.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps" RENAME ${PNG_NAME})
    install(FILES "../src/resources/logo/icon-512.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/512x512/apps" RENAME ${PNG_NAME})
endif()

message(STATUS "OK_CPACK=${OK_CPACK}")

if(LINUX AND OK_CPACK)
    ##---------------- 以下为用CPack打包成deb包的内容 ----------------##
    message(STATUS "--- CPACK ---")
    # 指定deb包的版本号
    set(CPACK_PACKAGE_VERSION_MAJOR "${OK_VER_MAJOR}")
    set(CPACK_PACKAGE_VERSION_MINOR "${OK_VER_MINOR}")
    set(CPACK_PACKAGE_VERSION_PATCH "${OK_VER_PATCH}")

    # https://cmake.org/cmake/help/book/mastering-cmake/chapter/Packaging%20With%20CPack.html#options-added-by-cpack
    set(CPACK_DESKTOP_GROUP "Network,Office")
    # 指定包名
    set(CPACK_PACKAGE_NAME ${APPLICATION_EXE_NAME})
    set(CPACK_PACKAGE_FILE_NAME ${APPLICATION_EXE_NAME}_${DISTRO_ID}-${DISTRO_VERSION_ID}_${CMAKE_SYSTEM_PROCESSOR})
    set(CPACK_PACKAGE_CONTACT ${OK_SUPPORT_EMAIL})
    set(CPACK_PACKAGE_HOMEPAGE_URL ${ORGANIZATION_HOME})
    set(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}/src/resources/logo/main.svg")
    set(CPACK_PACKAGE_VENDOR ${OK_MAINTAINER})
    set(CPACK_PACKAGE_MAINTAINER ${OK_MAINTAINER})
    set(CPACK_PACKAGE_SECTION ${CPACK_DESKTOP_GROUP})
    set(CPACK_PACKAGE_DESCRIPTION "OkMSG &#x652F;&#x6301;&#x72EC;&#x7ACB;&#x79C1;&#x6709;&#x5316;&#x90E8;&#x7F72;&#x7684;&#x96C6;&#x5373;&#x65F6;&#x6D88;&#x606F;&#x3001;&#x8BED;&#x97F3;&#x3001;&#x89C6;&#x9891;&#x901A;&#x8BDD;&#x3001;&#x53D1;&#x9001;&#x6587;&#x4EF6;&#x3001;&#x4F1A;&#x8BAE;&#x7B49;&#x591A;&#x79CD;&#x529F;&#x80FD;&#x4E8E;&#x4E00;&#x8EAB;&#x7684;&#x5F00;&#x6E90;&#x9879;&#x76EE;&#xFF0C;&#x540C;&#x65F6;&#x8BA9;&#x60A8;&#x7684;&#x4F01;&#x4E1A;&#x66F4;&#x52A0;&#x6709;&#x6548;&#x5F00;&#x542F;&#x534F;&#x4F5C;&#x3001;&#x6709;&#x6548;&#x6C9F;&#x901A;&#xFF0C;&#x63A7;&#x5236;&#x6210;&#x672C;&#xFF0C;&#x5F00;&#x62D3;&#x65B0;&#x4E1A;&#x52A1;&#xFF0C;&#x5E76;&#x5E2E;&#x52A9;&#x60A8;&#x52A0;&#x901F;&#x53D1;&#x5C55;&#x4E1A;&#x52A1;&#x3002;")

    if (${DISTRO_ID} MATCHES "ubuntu")
        set(CPACK_GENERATOR "DEB")
        set(OK_CPACK_DEPENDS "libvpx7,libx264-163,libx265-199,libpgm-5.3-0,libdc1394-25,libaom3,libdav1d5,libdouble-conversion3,libmysofa1")
        # DEB for ubuntu 22.04
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "${OK_CPACK_DEPENDS},\
            libqt5core5a,libqt5gui5,libqt5widgets5,\
            libqt5svg5,libqt5xml5,libqt5x11extras5,libqt5concurrent5,\
            libsdl2-2.0-0,libdbus-1-3,\
            libegl1,libfontconfig1,libfreetype6,libglib2.0-0,libglx0,libgl1,libglu1-mesa,\
            freeglut3,libgtk-3-0,libharfbuzz0b,libice6,libicu70,libopengl0,libpcre2-16-0,\
            libpng16-16,libsm6,libvulkan1,libx11-6,libx11-xcb1,libxcb1,libxcb-glx0,libxcb-icccm4,\
            libxcb-image0,libxcb-keysyms1,libxcb-randr0,libxcb-render0,libxcb-render-util0,\
            libxcb-shape0,libxcb-shm0,libxcb-sync1,libxcb-util1,libxcb-xfixes0,libxcb-xkb1,\
            libxcursor1,libxkbcommon0,libxkbcommon-x11-0,libxv1,libxss1,zlib1g,libflac8,\
            ocl-icd-libopencl1,libasound2,libasyncns0,libavc1394-0,libavcodec58,\
            libavdevice58,libavfilter7,libavformat58,libavutil56,libblas3,libbluray2,\
            libbs2b0,libcaca0,libcdio19,libcdio-cdda2,libcdio-paranoia2,libchromaprint1,\
            libcodec2-1.0,libdav1d5,libdecor-0-0,libexif12,libflite1,\
            libgfortran5,libgme0,libgomp1,libgsm1,libiec61883-0,libjack-jackd2-0,\
            liblapack3,liblilv-0-0,libmp3lame0,libmpg123-0,\
            libnorm1,libnuma1,libogg0,libopenal1,libopenjp2-7,libopenmpt0,libopus0,\
            libpocketsphinx3,libpostproc55,libpulse0,libqrencode4,libquadmath0,\
            libwebpmux3,libswscale5,libswresample3,\
            libsqlcipher0,libvorbis0a,libvorbisenc2,libvorbisfile3,libspeex1")

    elseif (${DISTRO_ID} MATCHES "debian")
        set(OK_CPACK_DEPENDS "libvpx5,libx264-155,libx265-165,libpgm-5.2-0,libdc1394-22,libaom0,libdavix0v5,libdouble-conversion1")
    elseif (${DISTRO_ID} MATCHES "fedora")
        # RPM for fedora 36
        set(CPACK_GENERATOR "RPM")
        set(CPACK_RPM_PACKAGE_GROUP ${CPACK_DESKTOP_GROUP})
        set(CPACK_RPM_PACKAGE_AUTOREQ ${OK_MAINTAINER})
        set(CPACK_RPM_PACKAGE_REQUIRES "qt5-qtbase,qt5-qtbase-gui,qt5-qtsvg,qt5-qtmultimedia,qt5-linguist,qt5-qttools,qt5-qtscxml,qt5-qttools-static,\
            libvpx,ffmpeg-free, libavcodec-free-devel, libavdevice-free,libexif,\
						qrencode-libs, \
            openal-soft,openssl-libs")
    endif ()

    # 指定需要安装的文件
    include(CPack)
    configure_file("../lib/xdg/${APPLICATION_EXE_NAME}.metainfo.xml.in" "${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_EXE_NAME}.metainfo.xml" @ONLY)
    generate_appdata_changelog(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/changelog.txt" "${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_EXE_NAME}.metainfo.xml")
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" BUNDLE DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(FILES "../lib/xdg/${APPLICATION_EXE_NAME}.desktop" DESTINATION "/usr/share/applications" RENAME "${APPLICATION_EXE_NAME}.desktop")
    install(FILES "../src/resources/logo/icon-16.png" DESTINATION "/usr/share/icons/hicolor/16x16/apps" RENAME ${PNG_NAME})
    install(FILES "../src/resources/logo/icon-32.png" DESTINATION "/usr/share/icons/hicolor/32x32/apps" RENAME ${PNG_NAME})
    install(FILES "../src/resources/logo/icon-48.png" DESTINATION "/usr/share/icons/hicolor/48x48/apps" RENAME ${PNG_NAME})
    install(FILES "../src/resources/logo/icon-64.png" DESTINATION "/usr/share/icons/hicolor/64x64/apps" RENAME ${PNG_NAME})
    install(FILES "../src/resources/logo/icon-128.png" DESTINATION "/usr/share/icons/hicolor/128x128/apps" RENAME ${PNG_NAME})
    install(FILES "../src/resources/logo/icon-256.png" DESTINATION "/usr/share/icons/hicolor/256x256/apps" RENAME ${PNG_NAME})
    install(FILES "../src/resources/logo/icon-512.png" DESTINATION "/usr/share/icons/hicolor/512x512/apps" RENAME ${PNG_NAME})
endif ()

if(LINUX AND PACMAN)
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)

    set(ICON_SIZES
        16x16
        24x24
        32x32
        48x48
        64x64
        72x72
        96x96
        128x128
        256x256)

    foreach(SIZE ${ICON_SIZES})
        set(ICON_FILE "icon/${SIZE}.png")
        set(DESTINATION_DIR "/usr/share/icons/hicolor/${SIZE}/apps")
        set(DESTINATION_PATH "${DESTINATION_DIR}/ok-msg.png")

        install(
            FILES ${CMAKE_CURRENT_BINARY_DIR}/${ICON_FILE}
            DESTINATION ${DESTINATION_DIR}
            PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

        install(
            CODE "execute_process(COMMAND xdg-icon-resource install --context apps --size ${SIZE} ${DESTINATION_PATH})"
        )
    endforeach()

    set(DESKTOP_FILE "ok-msg.desktop")
    set(DESTINATION_DIR "/usr/share/applications")

    configure_file(${DESKTOP_FILE}.in ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE}
        @ONLY)

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE}
        DESTINATION ${DESTINATION_DIR}
        PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
endif ()
