# === Gloox ===
FetchContent_Declare(gloox
    GIT_REPOSITORY https://github.com/okstar-org/ok-gloox.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(gloox)
message(STATUS "gloox_SOURCE_DIR=${gloox_SOURCE_DIR}")

add_subdirectory(network)
add_subdirectory(backend)
add_subdirectory(ortc)
add_subdirectory(messenger)
add_subdirectory(session)
add_subdirectory(settings)

if(ENABLE_PLUGINS)
  add_subdirectory(plugin)
endif()
