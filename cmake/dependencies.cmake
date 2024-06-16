# 配置工程信息
set(ORGANIZATION_NAME "OkStar")
set(ORGANIZATION_DOMAIN "okstar.org")
set(ORGANIZATION_HOME "https://github.com/okstar-org")
set(APPLICATION_ID "org.okstar.ok-msg-desktop")
set(APPLICATION_NAME "OkMSG-Desktop")
set(APPLICATION_ALIAS "OkMSG")
set(APPLICATION_EXE_NAME "ok-msg-desktop")
set(OK_SUPPORT_EMAIL "gaojie314@gmail.com")
set(OK_MAINTAINER ${ORGANIZATION_NAME})

add_definitions(
        -DORGANIZATION_NAME="${ORGANIZATION_NAME}"
        -DORGANIZATION_DOMAIN="${ORGANIZATION_DOMAIN}"
        -DORGANIZATION_HOME="${ORGANIZATION_HOME}"
        -DAPPLICATION_ID="${APPLICATION_ID}"
        -DAPPLICATION_NAME="${APPLICATION_NAME}"
        -DAPPLICATION_ALIAS="${APPLICATION_ALIAS}"
        -DAPPLICATION_EXE_NAME="${APPLICATION_EXE_NAME}"
)

# 设置Qt配置参数，默认从环境变量读取
#if (WIN32)
#    set(QT_DIR $ENV{QTDIR})
#    if (NOT DEFINED QT_DIR)
#        message(FATAL_ERROR "请在环境变量配置Qt路径【QTDIR】！")
#    endif ()
#    message(STATUS "QT_DIR=${QT_DIR}")
#    set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} ${QT_DIR})
#
#    # 设置Qt模块包含头文件和库
#    #include_directories(${CMAKE_PREFIX_PATH}/include)
#    #link_directories(${CMAKE_PREFIX_PATH}/lib)
#
#    # 根据Qt类型，设置动态(安装默认)或者静态(下载的静态版)，默认从环境变量读取
#    set(LINK_STATIC_QT $ENV{LINK_STATIC_QT})
#    if (NOT DEFINED LINK_STATIC_QT)
#        message(FATAL_ERROR "请在环境变量配置Qt类型【LINK_STATIC_QT】！")
#    endif ()
#    message(STATUS "LINK_STATIC_QT=${LINK_STATIC_QT}")
#endif ()
#

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

if (UNIX)
    include_directories(${Qt5LinuxAccessibilitySupport_INCLUDES})
    set(Qt5LinuxAccessibilitySupport_INCLUDES
            ${CMAKE_PREFIX_PATH}/include/QtLinuxAccessibilitySupport)
endif ()

# 开启插件（ON/OFF）
option(ENABLE_PLUGINS "Enable plugins" ON)
