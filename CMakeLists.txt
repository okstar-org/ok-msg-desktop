cmake_minimum_required(VERSION 3.21)

if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif(POLICY CMP0091)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif(POLICY CMP0135)
include(FetchContent)

project(OkMSG-Project)

# 在QtCreator下禁用CONAN
set(QT_CREATOR_SKIP_CONAN_SETUP TRUE)

# 取消CONAN对编译器的检查
set(CONAN_DISABLE_CHECK_COMPILER ON)

# Support for pthread.
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

include(ProcessorCount)
ProcessorCount(N)
message(STATUS "ProcessorCount=" ${N})

# 开启插件（ON/OFF）
set(ENABLE_PLUGINS ON)

message(STATUS "PROJECT_SOURCE_DIR=" ${PROJECT_SOURCE_DIR})
message(STATUS "CMAKE_CURRENT_SOURCE_DIR=" ${CMAKE_CURRENT_SOURCE_DIR})

message(STATUS "CMAKE_SYSTEM_PROCESSOR=" ${CMAKE_SYSTEM_PROCESSOR})
message(STATUS "CMAKE_HOST_SYSTEM=" ${CMAKE_HOST_SYSTEM_NAME} " " ${CMAKE_SYSTEM_VERSION})

execute_process(
        COMMAND git describe --tags
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_DESCRIBE
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

if(NOT GIT_DESCRIBE)
  set(GIT_DESCRIBE "Nightly")
endif()

add_definitions(-DGIT_DESCRIBE="${GIT_DESCRIBE}")

execute_process(
        COMMAND git rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_VERSION
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

add_definitions(-DGIT_VERSION="${GIT_VERSION}")
message(STATUS "GIT_VERSION=" ${GIT_VERSION})

if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64" OR CMAKE_SYSTEM_PROCESSOR STREQUAL "AMD64")
  set(ARCH "x64")
else()
  set(ARCH "x86")
endif()

if(LINUX)
  set(PLATFORM "linux")
elseif(WIN32)
  set(PLATFORM "windows")
elseif (APPLE)
  set(PLATFORM "macos")
endif()

message(STATUS "ARCH= " ${ARCH})
message(STATUS "PLATFORM= " ${PLATFORM} )

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# include(CheckAtomic)

# include(dependencies)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}"
        "${PROJECT_SOURCE_DIR}/cmake/modules")

# 设置编译器相关
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# set(CMAKE_PREFIX_PATH ${QT_DIR}) message(STATUS "CMAKE_PREFIX_PATH="
# ${CMAKE_PREFIX_PATH})

# For GNU/Linux and *BSD systems:
if(UNIX AND NOT(APPLE))
  set(LINUX ON)
endif()

if(UNIX)
  find_package(PkgConfig REQUIRED)
endif()

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

  # Windows暂时构建类型Release
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release;MinSizeRel;RelWithDebInfo")

  # Conan检查
  if(NOT EXISTS build/deps)
    execute_process(COMMAND conan install . -s compiler.runtime=MT
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

  # 设置相关宏定义
  add_definitions(-DWEBRTC_POSIX -DWEBRTC_LINUX -DQ_OS_POSIX
          -D_GLIBCXX_USE_CXX11_ABI=1 -DOPENSSL_IS_BORINGSSL=1)
elseif(APPLE)
  # do something related to APPLE
  message(ERROR "暂不支持 Not supported temporarily")
endif()

# conan
include_directories(build/deps/include)
link_directories(build/deps/lib)

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
  include(plugins/plugins.cmake)

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
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
        ${CMAKE_CURRENT_SOURCE_DIR}/config.h @ONLY NEWLINE_STYLE LF)
add_definitions(-DHAVE_CONFIG)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "CMAKE_BUILD_TYPE=" ${CMAKE_BUILD_TYPE})
string(TOLOWER ${CMAKE_BUILD_TYPE} LOWER_CMAKE_BUILD_TYPE)
if(CMAKE_BUILD_TYPE STREQUAL "release")
  add_definitions(-w)
endif()

# libc++ or libstdc++&clang
option(BUILD_WITH_LIBCXX "Build with libc++" OFF)
if(BUILD_WITH_LIBCXX AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  message(STATUS "Build with libc++")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++20")
  message(STATUS "Build with libstdc++")
endif()

# -Wunused-parameter -pedantic -fsanitize=address,undefined,leak,integer -Wextra
# -Wall -Wmacro-redefined -Wbuiltin-macro-redefined
#if(UNIX)
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all  -Wunused-function -Wstrict-overflow -Wstrict-aliasing -Wstack-protector")
#endif(UNIX)

if(CMAKE_C_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

  if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /W0")

    if(LINK_STATIC_QT)
      set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
      set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    else()
      set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
      set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    endif()
  endif()
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wextra -Wall")
elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /W0")

  if(LINK_STATIC_QT)
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG
            "${CMAKE_CXX_FLAGS_DEBUG} /MTd /D_ITERATOR_DEBUG_LEVEL=0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
  endif()
else()
  message(FATAL_ERROR "Not support compiler: ${CMAKE_C_COMPILER_ID} yet.")
endif()

message(STATUS "CMAKE_C_COMPILER_ID=" ${CMAKE_C_COMPILER_ID})
message(STATUS "CMAKE_C_COMPILER=" ${CMAKE_C_COMPILER})
message(STATUS "CMAKE_C_FLAGS=" ${CMAKE_C_FLAGS})
message(STATUS "CMAKE_C_STANDARD=" ${CMAKE_C_STANDARD})

message(STATUS "CMAKE_CXX_COMPILER_ID=" ${CMAKE_CXX_COMPILER_ID})
message(STATUS "CMAKE_CXX_COMPILER=" ${CMAKE_CXX_COMPILER})
message(STATUS "CMAKE_CXX_FLAGS=" ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_CXX_FLAGS_DEBUG=" ${CMAKE_CXX_FLAGS_DEBUG})
message(STATUS "CMAKE_CXX_FLAGS_RELEASE=" ${CMAKE_CXX_FLAGS_RELEASE})
message(STATUS "CMAKE_CXX_STANDARD=" ${CMAKE_CXX_STANDARD})

# 设置WebRTC位置
# set(WebRTC_DIR ${PROJECT_SOURCE_DIR}/3rdparty/webrtc/libwebrtc-${PLATFORM_ARCH})
include_directories(.)

add_subdirectory(src)

# Add RC files.
set(${PROJECT_NAME}_RC_FILES src/icon/icon.rc)

# Add QRC files.
set(${PROJECT_NAME}_QRC_FILES UI/resources/resources.qrc)

set(${PROJECT_NAME}_RESOURCES ${${PROJECT_NAME}_RC_FILES}
        ${${PROJECT_NAME}_QRC_FILES})

set(${PROJECT_NAME}_SOURCES src/main.cpp)

qt5_add_translation(${PROJECT_NAME}_QM_FILES translations/zh_CN.ts)

file(
        WRITE "${PROJECT_BINARY_DIR}/translations.qrc.in"
        "<!DOCTYPE RCC>
<RCC version=\"1.0\">
  <qresource prefix=\"/translations/${PROJECT_NAME}\">
")

foreach(qm ${${PROJECT_NAME}_QM_FILES})
  get_filename_component(qm_name ${qm} NAME)
  file(APPEND "${PROJECT_BINARY_DIR}/translations.qrc.in"
          "    <file alias=\"${qm_name}\">${qm}</file>\n")
endforeach(qm)

file(APPEND "${PROJECT_BINARY_DIR}/translations.qrc.in" "  </qresource>
</RCC>")

execute_process(
        COMMAND
        ${CMAKE_COMMAND} -E copy_if_different
        ${PROJECT_BINARY_DIR}/translations.qrc.in
        ${PROJECT_BINARY_DIR}/translations.qrc)

qt5_add_resources(${PROJECT_NAME}_RESOURCES
        ${PROJECT_BINARY_DIR}/translations.qrc)

if(WIN32)
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
elseif(UNIX)
  set(PLATFORM_LIBS z ssl crypto atomic)
endif()

set(ALL_LIBRARIES ${PLATFORM_LIBS})

if(ENABLE_MODULE_PAINTER)
  list(APPEND OK_LIBS Painter)
endif()

if(ENABLE_PLUGINS)
  list(APPEND OK_LIBS Plugin)
  include(${PROJECT_SOURCE_DIR}/plugins/plugins.cmake)
endif()

if(MSVC)
  set_property(
          TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY
          "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif(MSVC)

