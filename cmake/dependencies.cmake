# 设置Qt配置参数，默认从环境变量读取
if(WIN32)
    set(QT_DIR $ENV{QTDIR})
    if(NOT DEFINED QT_DIR)
        message(FATAL_ERROR "请在环境变量配置Qt路径【QTDIR】！")
    endif()
    message(STATUS "QT_DIR=${QT_DIR}")

    set(CMAKE_PREFIX_PATH ${QT_DIR})

    # Qt
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTORCC ON)

    find_package(Qt5 COMPONENTS Core
            Concurrent
            Widgets
            Gui
            Multimedia
            MultimediaWidgets
            Network
            Xml
            Sql
            Svg
            OpenGL
            LinguistTools
            UiTools
            REQUIRED)

    # 设置Qt模块包含头文件和库
    include_directories(${CMAKE_PREFIX_PATH}/include)
    link_directories(${CMAKE_PREFIX_PATH}/lib)
    option(PLATFORM_EXTENSIONS "Enable platform specific extensions, requires extra dependencies" ON)

    # 根据Qt类型，设置动态(安装默认)或者静态(下载的静态版)，默认从环境变量读取
    set(LINK_STATIC_QT $ENV{LINK_STATIC_QT})
    if(NOT DEFINED LINK_STATIC_QT)
        message(FATAL_ERROR "请在环境变量配置Qt类型【LINK_STATIC_QT】！")
    endif()
    message(STATUS "LINK_STATIC_QT=${LINK_STATIC_QT}")
endif()

if(UNIX)
    include_directories(${Qt5LinuxAccessibilitySupport_INCLUDES})
    set(Qt5LinuxAccessibilitySupport_INCLUDES
            ${CMAKE_PREFIX_PATH}/include/QtLinuxAccessibilitySupport)
endif()

# 开启插件（ON/OFF）
set(ENABLE_PLUGINS OFF)
