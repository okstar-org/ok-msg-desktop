# === Gloox ===
if(${DESKTOP_APP_USE_FLATPAK})
  message(STATUS "gloox_SOURCE_DIR=${gloox_SOURCE_DIR}")
  message(STATUS "gloox_LIB=${gloox_LIB}")
  include_directories(${gloox_SOURCE_DIR})
else ()
  find_package(ok-gloox REQUIRED)
  message(STATUS "Found ok-gloox=${ok-gloox_FOUND}")
  if(ok-gloox_FOUND)
    set(gloox_LIB ok-gloox::ok-gloox)
  endif ()
endif ()

# gloox
add_definitions(-DINBANDBYTESTREAM_TEST -DCLIENTBASE_TEST)

add_subdirectory(network)
add_subdirectory(backend)
add_subdirectory(ortc)
add_subdirectory(messenger)
add_subdirectory(session)
add_subdirectory(settings)
add_subdirectory(log)

if(ENABLE_PLUGINS)
  add_subdirectory(plugin)
endif()
