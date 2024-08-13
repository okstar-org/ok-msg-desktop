include(CMakeParseArguments)
function(search_dependency pkg)
  set(options OPTIONAL STATIC_PACKAGE)
  set(oneValueArgs PACKAGE LIBRARY FRAMEWORK HEADER)
  set(multiValueArgs)
  cmake_parse_arguments(arg "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Try pkg-config first.
  if (NOT ${pkg}_FOUND AND arg_PACKAGE)
    pkg_search_module(${pkg} ${arg_PACKAGE})
  endif ()

  # Then, try OSX frameworks.
  if (NOT ${pkg}_FOUND AND arg_FRAMEWORK)

      find_library(
        ${pkg}_LIBRARIES
        NAMES ${arg_FRAMEWORK}
        PATHS ${CMAKE_OSX_SYSROOT}/System/Library
        PATH_SUFFIXES Frameworks
        NO_DEFAULT_PATH)
    if (${pkg}_LIBRARIES)
      set(${pkg}_FOUND TRUE)
    endif ()
  endif ()

  # Last, search for the library itself globally.
  if (NOT ${pkg}_FOUND AND arg_LIBRARY)
    find_library(${pkg}_LIBRARIES NAMES ${arg_LIBRARY})
    if (arg_HEADER)
      find_path(${pkg}_INCLUDE_DIRS NAMES ${arg_HEADER})
    endif ()
    if (${pkg}_LIBRARIES AND (${pkg}_INCLUDE_DIRS OR NOT arg_HEADER))
      set(${pkg}_FOUND TRUE)
    endif ()
  endif ()

  if (NOT ${pkg}_FOUND)
    if (NOT arg_OPTIONAL)
      message(FATAL_ERROR "${pkg} package, library or framework not found")
    else ()
      message(STATUS "${pkg} not found")
    endif ()
  else ()
#    if (arg_STATIC_PACKAGE)
#      set(maybe_static _STATIC)
#    else ()
#      set(maybe_static "")
#    endif ()


    message(STATUS ${pkg} " LIBRARY_DIRS: ${${pkg}${maybe_static}_LIBRARY_DIRS}")
    message(STATUS ${pkg} " INCLUDE_DIRS: ${${pkg}${maybe_static}_INCLUDE_DIRS}")
    message(STATUS ${pkg} " CFLAGS_OTHER: ${${pkg}${maybe_static}_CFLAGS_OTHER}")
    message(STATUS ${pkg} " LIBRARIES:    ${${pkg}${maybe_static}_LIBRARIES}")

    link_directories(${${pkg}${maybe_static}_LIBRARY_DIRS} )
    include_directories(${${pkg}${maybe_static}_INCLUDE_DIRS} )

    foreach (flag ${${pkg}${maybe_static}_CFLAGS_OTHER})
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}" PARENT_SCOPE)
    endforeach ()

    set(ALL_LINK_DIRS ${ALL_LINK_DIRS} ${${pkg}${maybe_static}_LIBRARY_DIRS} PARENT_SCOPE)
    set(ALL_LIBRARIES ${ALL_LIBRARIES} ${${pkg}${maybe_static}_LIBRARIES} PARENT_SCOPE)
    message(STATUS "${pkg} found")
  endif ()

  set(${pkg}_FOUND ${${pkg}_FOUND} PARENT_SCOPE)
endfunction()


