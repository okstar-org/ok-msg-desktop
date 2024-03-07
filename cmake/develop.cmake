# 开启插件（ON/OFF）
option(ENABLE_PLUGINS "Enable plugins" ON)

if(ENABLE_PLUGINS)
    message(STATUS "ENABLE_PLUGINS")
endif()

# 在QtCreator下禁用CONAN
set(QT_CREATOR_SKIP_CONAN_SETUP TRUE)
# 取消CONAN对编译器的检查
set(CONAN_DISABLE_CHECK_COMPILER ON)

include(ProcessorCount)
ProcessorCount(N)
message(STATUS "ProcessorCount=" ${N})

message(STATUS "CMAKE_SOURCE_DIR=" ${CMAKE_SOURCE_DIR})
message(STATUS "PROJECT_SOURCE_DIR=" ${PROJECT_SOURCE_DIR})
message(STATUS "PROJECT_BINARY_DIR=" ${PROJECT_BINARY_DIR})
message(STATUS "CMAKE_CURRENT_SOURCE_DIR=" ${CMAKE_CURRENT_SOURCE_DIR})
message(STATUS "CMAKE_HOST_SYSTEM_NAME=" ${CMAKE_HOST_SYSTEM_NAME})
message(STATUS "CMAKE_GENERATOR_PLATFORM=" ${CMAKE_GENERATOR_PLATFORM})
message(STATUS "CMAKE_SYSTEM_PROCESSOR=" ${CMAKE_SYSTEM_PROCESSOR})
message(STATUS "CMAKE_SYSTEM_VERSION=" ${CMAKE_SYSTEM_VERSION})

execute_process(
  COMMAND git describe --tags
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_DESCRIBE
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

if(NOT GIT_DESCRIBE)
  set(GIT_DESCRIBE "Nightly")
endif()

add_definitions(-DGIT_DESCRIBE="${GIT_DESCRIBE}")

execute_process(
  COMMAND git rev-parse HEAD
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_VERSION
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

add_definitions(-DGIT_VERSION="${GIT_VERSION}")
message(STATUS "GIT_VERSION=" ${GIT_VERSION})

set(OK_ARCH ${CMAKE_SYSTEM_PROCESSOR})
set(OK_PLATFORM ${CMAKE_C_PLATFORM_ID})

set(PLATFORM_ARCH ${CMAKE_C_PLATFORM_ID}-${CMAKE_SYSTEM_PROCESSOR})
message(STATUS "PLATFORM_ARCH=" ${PLATFORM_ARCH})

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
# include(CheckAtomic)

# include(dependencies)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}"
     "${PROJECT_SOURCE_DIR}/cmake/modules")

# set(CMAKE_PREFIX_PATH ${QT_DIR}) message(STATUS "CMAKE_PREFIX_PATH="
# ${CMAKE_PREFIX_PATH})

# Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(
  Qt5
  COMPONENTS Core
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
             DBus
             LinguistTools
             UiTools
  REQUIRED)

option(PLATFORM_EXTENSIONS "Enable platform specific extensions, requires extra dependencies" ON)

# 设置Qt模块包含头文件和库
include_directories(${CMAKE_PREFIX_PATH}/include)
link_directories(${CMAKE_PREFIX_PATH}/lib)

# set(QtModules Core Widgets Gui Multimedia MultimediaWidgets Network Concurrent
# Sql Svg Xml XmlPatterns OpenGL ANGLE) if(UNIX) set(QtModules ${QtModules}
# DBus) endif(UNIX)
#
# foreach (Module ${QtModules}) set(Qt${Module}${QT_VERSION_MAJOR}_INCLUDES
# ${CMAKE_PREFIX_PATH}/include/Qt${Module})
# include_directories(${Qt${Module}${QT_VERSION_MAJOR}_INCLUDES}) endforeach
# (Module)

if(WIN32)
  set(ARCH "x64")
  string(TOLOWER ${CMAKE_C_PLATFORM_ID}-${ARCH} PLATFORM_ARCH)
  message(STATUS "PLATFORM_ARCH=" ${PLATFORM_ARCH})

  # Conan检查
  if(NOT EXISTS build/deps)
    execute_process(COMMAND conan install ${PROJECT_SOURCE_DIR} -s compiler.runtime=MT
                            --build=missing)
  endif()

  # 设置相关宏定义
  add_definitions(
    -DWEBRTC_WIN
    # windows预处理宏
    -DNOMINMAX
    -DWIN32_LEAN_AND_MEAN
    -DWINDOWS
    -DNDEBUG
    # OpenAL
    -DAL_LIBTYPE_STATIC)

  if(MSVC)
    option(USE_MP "use multiple" ON)
    option(ProjectConfig_Global_COMPILE_FLAGS_WITH_MP
           "Set The Global Option COMPILE_FLAGS /MP to target." ON)
    if(ProjectConfig_Global_COMPILE_FLAGS_WITH_MP OR USE_MP)
      set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    endif()
    set(VS_STARTUP_PROJECT ${PROJECT_NAME})
  endif(MSVC)

elseif(LINUX)
  string(TOLOWER ${PLATFORM_ARCH} PLATFORM_ARCH)

  # 设置相关宏定义
  add_definitions(-DWEBRTC_POSIX -DWEBRTC_LINUX -DQ_OS_POSIX
                  -D_GLIBCXX_USE_CXX11_ABI=1 -DOPENSSL_IS_BORINGSSL=1)
elseif(APPLE)
  # do something related to APPLE
  message(ERROR "暂不支持 Not supported temporarily")
endif()

# conan
include_directories(${PROJECT_SOURCE_DIR}/build/deps/include)
link_directories(${PROJECT_SOURCE_DIR}/build/deps/lib)

# Gloox
add_definitions(
  # -DQSSLSOCKET_DEBUG
  -DLOG_XMPP # xmpp logs
  -DWANT_PING # xmpp ping
  -DENABLE_SEND_RAW_XML # xmpp send raw xmls
  -DLOG_TO_FILE
  -DHAVE_OPENSSL)

if(ENABLE_PLUGINS)
  add_definitions(-DOK_PLUGIN)
  include(${PROJECT_SOURCE_DIR}/plugins/plugins.cmake)

  if(NOT DEV_MODE)
    set(LIB_SUFFIX
        ""
        CACHE STRING "Define suffix of directory name")
    set(OK_LIBDIR
        "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/${PROJECT_NAME}"
        CACHE STRING "${PROJECT_NAME} libraries directory")
    set(OK_DATADIR
        "${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}"
        CACHE STRING "${PROJECT_NAME} data directory")
  else()
    set(OK_LIBDIR ".")
    set(OK_DATADIR ".")
  endif()
endif()

if(ENABLE_MODULE_PAINTER)
  add_definitions(-DOK_MODULE_PAINTER)
endif()

# config.h.in -> config.h
configure_file(${PROJECT_SOURCE_DIR}/config.h.in
               ${PROJECT_BINARY_DIR}/config.h @ONLY NEWLINE_STYLE LF)
include_directories(${PROJECT_BINARY_DIR})

add_definitions(-DHAVE_CONFIG)

if(UNIX)
  include_directories(${Qt5LinuxAccessibilitySupport_INCLUDES})
  set(Qt5LinuxAccessibilitySupport_INCLUDES
      ${CMAKE_PREFIX_PATH}/include/QtLinuxAccessibilitySupport)
endif()

# 设置WebRTC位置
#set(WebRTC_DIR ${PROJECT_SOURCE_DIR}/3rdparty/webrtc/libwebrtc-${PLATFORM_ARCH})