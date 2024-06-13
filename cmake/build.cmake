set(CMAKE_CONFIGURATION_TYPES Debug;Release)


# Compile Standard
set(CMAKE_C_STANDARD 11)
message(STATUS "CMAKE_C_STANDARD=${CMAKE_C_STANDARD}")
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20)
message(STATUS "CMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Build Type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()
string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-DLOG_TO_FILE=1)
endif ()

# Support for pthread
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

set(BUILD_SHARED_LIBS OFF)

if (WIN32)
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_ITERATOR_DEBUG_LEVEL=0" )
    # add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)
endif ()


if(UNIX)
    find_package(PkgConfig REQUIRED)
    # -Wunused-parameter -pedantic -fsanitize=address,undefined,leak,integer -Wextra
    # -Wall -Wmacro-redefined -Wbuiltin-macro-redefined
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-all -Wunused-function -Wstrict-overflow -Wstrict-aliasing -Wstack-protector")
endif(UNIX)

if(MSVC)
    option(USE_MP "use multiple" ON)
    option(ProjectConfig_Global_COMPILE_FLAGS_WITH_MP "Set The Global Option COMPILE_FLAGS /MP to target." ON)
    if(ProjectConfig_Global_COMPILE_FLAGS_WITH_MP OR USE_MP)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP ")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP ")
    endif()
    #set(VS_STARTUP_PROJECT ${PROJECT_NAME})
    #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DNDEBUG")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

message(STATUS "CMAKE_C_COMPILER_ID=" ${CMAKE_C_COMPILER_ID})
message(STATUS "CMAKE_C_COMPILER=" ${CMAKE_C_COMPILER})
message(STATUS "CMAKE_C_FLAGS=" ${CMAKE_C_FLAGS})

message(STATUS "CMAKE_CXX_COMPILER_ID=" ${CMAKE_CXX_COMPILER_ID})
message(STATUS "CMAKE_CXX_COMPILER=" ${CMAKE_CXX_COMPILER})
message(STATUS "CMAKE_CXX_FLAGS=" ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_CXX_FLAGS_DEBUG=" ${CMAKE_CXX_FLAGS_DEBUG})
message(STATUS "CMAKE_CXX_FLAGS_RELEASE=" ${CMAKE_CXX_FLAGS_RELEASE})
