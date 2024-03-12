# For GNU/Linux and *BSD systems:
if(UNIX AND NOT (APPLE))
  set(LINUX ON)
endif()

if(UNIX)
  find_package(PkgConfig REQUIRED)
endif()

# -Wunused-parameter -pedantic -fsanitize=address,undefined,leak,integer -Wextra
# -Wall -Wmacro-redefined -Wbuiltin-macro-redefined
if(UNIX)
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -fstack-protector-all  -Wunused-function -Wstrict-overflow -Wstrict-aliasing -Wstack-protector"
  )
endif(UNIX)

message(STATUS "CMAKE_C_COMPILER_ID=" ${CMAKE_C_COMPILER_ID})
message(STATUS "CMAKE_C_COMPILER=" ${CMAKE_C_COMPILER})
message(STATUS "CMAKE_C_FLAGS=" ${CMAKE_C_FLAGS})

message(STATUS "CMAKE_CXX_COMPILER_ID=" ${CMAKE_CXX_COMPILER_ID})
message(STATUS "CMAKE_CXX_COMPILER=" ${CMAKE_CXX_COMPILER})
message(STATUS "CMAKE_CXX_FLAGS=" ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_CXX_FLAGS_DEBUG=" ${CMAKE_CXX_FLAGS_DEBUG})
message(STATUS "CMAKE_CXX_FLAGS_RELEASE=" ${CMAKE_CXX_FLAGS_RELEASE})

if (MSVC)
  MESSAGE(STATUS "MSVC_RUNTIME_LIBRARY=MultiThreaded$<$<CONFIG:Debug>:Debug>")
  set_property( GLOBAL
         PROPERTY MSVC_RUNTIME_LIBRARY
          "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif (MSVC)