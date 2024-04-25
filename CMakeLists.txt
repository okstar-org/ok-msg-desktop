cmake_minimum_required(VERSION 3.22)
cmake_policy(SET CMP0076 NEW)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0135 NEW)

include(CMakeDependentOption)
include(CheckSymbolExists)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

project(OkMSG-Project
    LANGUAGES C CXX ASM
    DESCRIPTION "OkMSG"
    HOMEPAGE_URL "https://github.com/okstar-org/ok-msg-desktop.git"
)

include(cmake/init_target.cmake)
include(cmake/generate_target.cmake)
include(cmake/generate_appdata_changelog.cmake)
include(cmake/dependencies.cmake)
include(cmake/platform.cmake)
include(cmake/build.cmake)
include(cmake/develop.cmake)

include_directories(.)

add_subdirectory(src)

if(ENABLE_PLUGINS)
  include(${PROJECT_SOURCE_DIR}/plugins/plugins.cmake)
endif()

