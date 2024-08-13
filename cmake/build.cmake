# Compile Standard
set(CMAKE_C_STANDARD 11)
message(STATUS "CMAKE_C_STANDARD=${CMAKE_C_STANDARD}")
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20)
message(STATUS "CMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Build Type
message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-DLOG_TO_FILE=1)
endif ()

# Support for pthread
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

set(BUILD_SHARED_LIBS OFF)

if (WIN32)
    add_definitions(-DWIN32_LEAN_AND_MEAN=1)
    if(CMAKE_BUILD_TYPE MATCHES Release)
        add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)
    elseif(CMAKE_BUILD_TYPE MATCHES Debug)
        add_definitions(-D_ITERATOR_DEBUG_LEVEL=2)        
    endif()
endif ()


if(LINUX)
    find_package(PkgConfig REQUIRED)
    # -Wunused-parameter -pedantic -fsanitize=address,undefined,leak,integer -Wextra
    # -Wall -Wmacro-redefined -Wbuiltin-macro-redefined
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -Wunused-function -Wstrict-overflow -Wstrict-aliasing -Wstack-protector")
endif(LINUX)

if(MSVC)
    option(USE_MP "use multiple" ON)
    option(ProjectConfig_Global_COMPILE_FLAGS_WITH_MP "Set The Global Option COMPILE_FLAGS /MP to target." ON)
    
    if(ProjectConfig_Global_COMPILE_FLAGS_WITH_MP OR USE_MP)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP ")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP ")
    endif()

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /DNDEBUG")


endif()


message(STATUS "CMAKE_C_COMPILER_ID=" ${CMAKE_C_COMPILER_ID})
message(STATUS "CMAKE_C_COMPILER=" ${CMAKE_C_COMPILER})
message(STATUS "CMAKE_C_FLAGS=" ${CMAKE_C_FLAGS})

message(STATUS "CMAKE_CXX_COMPILER_ID=" ${CMAKE_CXX_COMPILER_ID})
message(STATUS "CMAKE_CXX_COMPILER=" ${CMAKE_CXX_COMPILER})
message(STATUS "CMAKE_CXX_FLAGS=" ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_CXX_FLAGS_DEBUG=" ${CMAKE_CXX_FLAGS_DEBUG})
message(STATUS "CMAKE_CXX_FLAGS_RELEASE=" ${CMAKE_CXX_FLAGS_RELEASE})


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