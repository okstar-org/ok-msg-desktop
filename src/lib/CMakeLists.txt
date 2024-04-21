# === Gloox ===
if(${DESKTOP_APP_USE_FLATPAK})
  message(STATUS "gloox_SOURCE_DIR=${gloox_SOURCE_DIR}")
  message(STATUS "gloox_LIB=${gloox_LIB}")
  include_directories(${gloox_SOURCE_DIR})
else ()
  FetchContent_Declare(gloox
      GIT_REPOSITORY https://github.com/okstar-org/ok-gloox.git
      GIT_TAG        main
  )
  FetchContent_MakeAvailable(gloox)
  include_directories(${gloox_SOURCE_DIR}/src)
  set(gloox_LIB gloox)
  message(STATUS "gloox_SOURCE_DIR=${gloox_SOURCE_DIR}")
  message(STATUS "gloox_LIB=${gloox_LIB}")
endif ()

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
