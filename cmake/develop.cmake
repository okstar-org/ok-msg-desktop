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


option(PLATFORM_EXTENSIONS "Enable platform specific extensions, requires extra dependencies" ON)

# config.h.in -> config.h
configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_BINARY_DIR}/config.h @ONLY NEWLINE_STYLE LF)
include_directories(${PROJECT_BINARY_DIR})
add_definitions(-DHAVE_CONFIG)


# 开启插件（ON/OFF）
option(ENABLE_PLUGINS "Enable plugins" ON)
message(STATUS "ENABLE_PLUGINS=${ENABLE_PLUGINS}")

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
