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

set(${PROJECT_NAME}_RESOURCES ${${PROJECT_NAME}_RC_FILES} ${${PROJECT_NAME}_QRC_FILES})

set(${PROJECT_NAME}_SOURCES
        main.cpp
        application.cpp
        launcher.cpp
)

list(APPEND OK_LIBS
        UI
        IM
        OkIMCore
        OkSession
        OkSettings
        OkNetwork
        OkBackend
        OkBase
)

if(ENABLE_PLUGINS)
    list(APPEND OK_LIBS Plugin)
endif()

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
        PRIVATE ${Qt5Core_LIBRARIES}
        PRIVATE ${Qt5Gui_LIBRARIES}
        PRIVATE ${Qt5Network_LIBRARIES}
        PRIVATE ${Qt5Widgets_LIBRARIES}
        PRIVATE ${Qt5MultimediaWidgets_LIBRARIES}
        PRIVATE ${Qt5Multimedia_LIBRARIES}
        PRIVATE ${Qt5Xml_LIBRARIES}
        PRIVATE ${Qt5Svg_LIBRARIES}
)

if (MSVC)
    set_property(
            TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif (MSVC)

if (LINUX AND DESKTOP_APP_USE_PACKAGED)
    message(STATUS "CMAKE_INSTALL_DATAROOTDIR=${CMAKE_INSTALL_DATAROOTDIR}")
    message(STATUS "CMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_BINDIR}")

    set(PNG_NAME "ok-msg.png")
    include(GNUInstallDirs)
    configure_file("../lib/xdg/${APPLICATION_EXE_NAME}.metainfo.xml.in" "${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_EXE_NAME}.metainfo.xml" @ONLY)
    generate_appdata_changelog(${PROJECT_NAME} "${CMAKE_SOURCE_DIR}/changelog.txt" "${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_EXE_NAME}.metainfo.xml")
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" BUNDLE DESTINATION "${CMAKE_INSTALL_BINDIR}")
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${APPLICATION_EXE_NAME}.metainfo.xml" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/metainfo" RENAME "${DESKTOP_LAUNCHER_BASENAME}.metainfo.xml")
    install(FILES "../lib/xdg/${APPLICATION_EXE_NAME}.desktop" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/applications" RENAME "${DESKTOP_LAUNCHER_BASENAME}.desktop")
    install(FILES "../src/UI/resources/logo/icon-16.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/16x16/apps" RENAME ${PNG_NAME})
    install(FILES "../src/UI/resources/logo/icon-32.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/32x32/apps" RENAME ${PNG_NAME})
    install(FILES "../src/UI/resources/logo/icon-48.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/48x48/apps" RENAME ${PNG_NAME})
    install(FILES "../src/UI/resources/logo/icon-64.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/64x64/apps" RENAME ${PNG_NAME})
    install(FILES "../src/UI/resources/logo/icon-128.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/128x128/apps" RENAME ${PNG_NAME})
    install(FILES "../src/UI/resources/logo/icon-256.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/256x256/apps" RENAME ${PNG_NAME})
    install(FILES "../src/UI/resources/logo/icon-512.png" DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/512x512/apps" RENAME ${PNG_NAME})
endif()


##---------------- 以下为用CPack打包成deb包的内容 ----------------##
# 指定deb包的版本号
set(_VERSION_MAJOR 1)
set(_VERSION_MINOR 2)
set(_VERSION_PATCH 3)
set(CPACK_PACKAGE_VERSION_MAJOR "${_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${_VERSION_PATCH}")
# 指定CPack生成的是deb包（还可以是其他包如 <.7z> <.zip> 等）
set(CPACK_GENERATOR "DEB")
# 指定包名
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
# 指定程序名，就是程序安装后的名字
set(CPACK_DEBIAN_PACKAGE_NAME ${APPLICATION_ID})
# 指定运行的计算机架构
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${CMAKE_C_LIBRARY_ARCHITECTURE})
# 指定所需的依赖
# set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgrpc++")
# 设置包作者的联系方式
set(CPACK_PACKAGE_CONTACT ${SUPPORT_EMAIL})
# 设置维护人
set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${SUPPORT_EMAIL})

# 指定需要安装的文件
INSTALL(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION /tmp/${APPLICATION_ID}
)

include(CPack)

