# Copyright (c) 2022 船山信息 chuanshaninfo.com This project is licensed under
# Mulan PubL v2. You can use this software according to the terms and conditions
# of the Mulan PubL v2. You may obtain a copy of Mulan PubL v2 at:
# http://license.coscl.org.cn/MulanPubL-2.0 THIS SOFTWARE IS PROVIDED ON AN "AS
# IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A
# PARTICULAR PURPOSE. See the Mulan PubL v2 for more details.
#
# Mulan Public License，Version 2
#
# Mulan Public License，Version 2 (Mulan PubL v2)
#
# May 2021 http://license.coscl.org.cn/MulanPubL-2.0

# ##############################################################################
#
# :: Dependencies
#
# ##############################################################################

include(CMakeParseArguments)
# include(Qt5CorePatches)

find_package(PkgConfig REQUIRED)

function(search_dependency pkg)
  set(options OPTIONAL STATIC_PACKAGE)
  set(oneValueArgs PACKAGE LIBRARY FRAMEWORK HEADER)
  set(multiValueArgs)
  cmake_parse_arguments(arg "${options}" "${oneValueArgs}" "${multiValueArgs}"
      ${ARGN})

  # Try pkg-config first.
  if(NOT ${pkg}_FOUND AND arg_PACKAGE)
    pkg_search_module(${pkg} ${arg_PACKAGE})
  endif()

  # Then, try OSX frameworks.
  if(NOT ${pkg}_FOUND AND arg_FRAMEWORK)
    find_library(
        ${pkg}_LIBRARIES
        NAMES ${arg_FRAMEWORK}
        PATHS ${CMAKE_OSX_SYSROOT}/System/Library
        PATH_SUFFIXES Frameworks
        NO_DEFAULT_PATH)
    if(${pkg}_LIBRARIES)
      set(${pkg}_FOUND TRUE)
    endif()
  endif()

  # Last, search for the library itself globally.
  if(NOT ${pkg}_FOUND AND arg_LIBRARY)
    find_library(${pkg}_LIBRARIES NAMES ${arg_LIBRARY})
    if(arg_HEADER)
      find_path(${pkg}_INCLUDE_DIRS NAMES ${arg_HEADER})
    endif()
    if(${pkg}_LIBRARIES AND (${pkg}_INCLUDE_DIRS OR NOT arg_HEADER))
      set(${pkg}_FOUND TRUE)
    endif()
  endif()

  if(NOT ${pkg}_FOUND)
    if(NOT arg_OPTIONAL)
      message(FATAL_ERROR "${pkg} package, library or framework not found")
    else()
      message(STATUS "${pkg} not found")
    endif()
  else()
    if(arg_STATIC_PACKAGE)
      set(maybe_static _STATIC)
    else()
      set(maybe_static "")
    endif()

    message(STATUS ${pkg} " LIBRARY_DIRS: "
        "${${pkg}${maybe_static}_LIBRARY_DIRS}")
    message(STATUS ${pkg} " INCLUDE_DIRS: "
        "${${pkg}${maybe_static}_INCLUDE_DIRS}")
    message(STATUS ${pkg} " CFLAGS_OTHER: "
        "${${pkg}${maybe_static}_CFLAGS_OTHER}")
    message(STATUS ${pkg} " LIBRARIES:    "
        "${${pkg}${maybe_static}_LIBRARIES}")

    link_directories(${${pkg}${maybe_static}_LIBRARY_DIRS})
    include_directories(${${pkg}${maybe_static}_INCLUDE_DIRS})

    foreach(flag ${${pkg}${maybe_static}_CFLAGS_OTHER})
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
    endforeach()

    set(ALL_LIBRARIES
        ${ALL_LIBRARIES} ${${pkg}${maybe_static}_LIBRARIES}
        PARENT_SCOPE)
    message(STATUS "${pkg} found")
  endif()

  set(${pkg}_FOUND
      ${${pkg}_FOUND}
      PARENT_SCOPE)
endfunction()

search_dependency(LIBAVDEVICE PACKAGE libavdevice)
search_dependency(LIBAVCODEC PACKAGE libavcodec)
search_dependency(LIBAVFORMAT PACKAGE libavformat)
search_dependency(LIBAVUTIL PACKAGE libavutil)
search_dependency(LIBEXIF  PACKAGE libexif)
search_dependency(LIBQRENCODE PACKAGE libqrencode)
search_dependency(LIBSWSCALE PACKAGE libswscale)
search_dependency(LIBSQLITE PACKAGE sqlite3)


if(${SPELL_CHECK})
  find_package(KF5Sonnet)
  if(KF5Sonnet_FOUND)
    add_definitions(-DSPELL_CHECKING)
    add_dependency(KF5::SonnetUi)
  else()
    message(WARNING "Sonnet not found. Spell checking will be disabled.")
  endif()
endif()

search_dependency(OPENAL PACKAGE openal)

if(PLATFORM_EXTENSIONS
    AND UNIX
    AND NOT APPLE)
  # Automatic auto-away support. (X11 also using for capslock detection)
  search_dependency(X11 PACKAGE x11 OPTIONAL)
  search_dependency(XSS PACKAGE xscrnsaver OPTIONAL)
endif()

if(APPLE)
  search_dependency(AVFOUNDATION FRAMEWORK AVFoundation)
  search_dependency(COREMEDIA FRAMEWORK CoreMedia)
  search_dependency(COREGRAPHICS FRAMEWORK CoreGraphics)
  search_dependency(FOUNDATION FRAMEWORK Foundation OPTIONAL)
  search_dependency(IOKIT FRAMEWORK IOKit OPTIONAL)

endif()


if(WIN32)
  set(ALL_LIBRARIES ${ALL_LIBRARIES} strmiids)
  # Qt doesn't provide openssl on windows
  search_dependency(OPENSSL PACKAGE openssl)
else ()
  find_package(OpenSSL)
endif()

set(APPLE_EXT False)
if(FOUNDATION_FOUND AND IOKIT_FOUND)
  set(APPLE_EXT True)
endif()

set(X11_EXT False)
if(X11_FOUND AND XSS_FOUND)
  set(X11_EXT True)
endif()

if(PLATFORM_EXTENSIONS)
  if(${APPLE_EXT}
      OR ${X11_EXT}
      OR WIN32)
    add_definitions(-DQTOX_PLATFORM_EXT)
    message(STATUS "Using platform extensions")
  else()
    message(WARNING "Not using platform extensions, dependencies not found")
    set(PLATFORM_EXTENSIONS OFF)
  endif()
endif()

if(${DESKTOP_NOTIFICATIONS})
  # snorenotify does only provide a cmake find module
  find_package(LibsnoreQt5 0.7.0 REQUIRED)
  set(ALL_LIBRARIES ${ALL_LIBRARIES} Snore::Libsnore)
endif()
