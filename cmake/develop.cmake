# 开启插件（ON/OFF）
option(ENABLE_PLUGINS "Enable plugins" ON)

if(ENABLE_PLUGINS)
    message(STATUS "ENABLE_PLUGINS")
endif()

include(ProcessorCount)
ProcessorCount(N)
message(STATUS "ProcessorCount=" ${N})

message(STATUS "PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}")
message(STATUS "PROJECT_BINARY_DIR=${PROJECT_BINARY_DIR}")
message(STATUS "CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}")

execute_process(
  COMMAND git rev-parse HEAD
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_VERSION
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

add_definitions(-DGIT_VERSION="${GIT_VERSION}")
message(STATUS "GIT_VERSION=" ${GIT_VERSION})

execute_process(
    COMMAND git describe --tags
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DESCRIBE
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Fetch GIT_DESCRIBE=${GIT_DESCRIBE}")

if (NOT GIT_DESCRIBE)
  set(GIT_DESCRIBE "v0.0.0")
  message(STATUS "Set GIT_DESCRIBE=${GIT_DESCRIBE}")
endif ()

add_definitions(-DGIT_DESCRIBE="${GIT_DESCRIBE}")

string(REPLACE "-" ";" VERSION_ARRAY ${GIT_DESCRIBE})
list(GET VERSION_ARRAY 0 GIT_TAG)
message(STATUS "GIT_TAG=${GIT_TAG}")

string(REPLACE "v" "" GIT_VER ${GIT_TAG})
message(STATUS "GIT_VER=${GIT_VER}")

# extra GIT VERSION
string(REPLACE "." ";" GIT_VER_SEQ ${GIT_VER})
list(LENGTH GIT_VER_SEQ GIT_VER_LEN)
if (NOT ${GIT_VER_LEN} EQUAL 3)
  message(FATAL_ERROR "Invalid version:${GIT_VER}")
endif ()

list(GET GIT_VER_SEQ 0 OK_VER_MAJOR)
list(GET GIT_VER_SEQ 1 OK_VER_MINOR)
list(GET GIT_VER_SEQ 2 OK_VER_PATCH)
message(STATUS "OK_VER_MAJOR=${OK_VER_MAJOR}")
message(STATUS "OK_VER_MINOR=${OK_VER_MINOR}")
message(STATUS "OK_VER_PATCH=${OK_VER_PATCH}")

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
# include(CheckAtomic)

# include(dependencies)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}"
     "${PROJECT_SOURCE_DIR}/cmake/modules")

# set(CMAKE_PREFIX_PATH ${QT_DIR}) message(STATUS "CMAKE_PREFIX_PATH="
# ${CMAKE_PREFIX_PATH})


option(PLATFORM_EXTENSIONS "Enable platform specific extensions, requires extra dependencies" ON)

# config.h.in -> config.h
configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/ok_config.h @ONLY NEWLINE_STYLE LF)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_definitions(-DOK_HAVE_CONFIG)

set(QT_CREATOR_SKIP_VCPKG_SETUP ON)

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
