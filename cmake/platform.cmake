message(STATUS "CMAKE_HOST_SYSTEM_NAME=" ${CMAKE_HOST_SYSTEM_NAME})
message(STATUS "CMAKE_GENERATOR_PLATFORM=" ${CMAKE_GENERATOR_PLATFORM})
message(STATUS "CMAKE_SYSTEM_PROCESSOR=" ${CMAKE_SYSTEM_PROCESSOR})
message(STATUS "CMAKE_SYSTEM_VERSION=" ${CMAKE_SYSTEM_VERSION})

cmake_host_system_information(RESULT PRETTY_NAME QUERY DISTRIB_PRETTY_NAME)
message(STATUS "OS Information:${PRETTY_NAME}")

cmake_host_system_information(RESULT DISTRO QUERY DISTRIB_INFO)
foreach(VAR IN LISTS DISTRO)
    message(STATUS "\t ${VAR}=`${${VAR}}`")
endforeach()


if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    message(STATUS "This is Windows")
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(LINUX ON)
    message(STATUS "This is Linux")
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    message(STATUS "This is macOS")
endif()

if(WIN32)
    set(ARCH "x64")
    string(TOLOWER ${CMAKE_C_PLATFORM_ID}-${ARCH} PLATFORM_ARCH)
    message(STATUS "PLATFORM_ARCH=" ${PLATFORM_ARCH})

    # 设置相关宏定义
    add_definitions(
            -DWEBRTC_WIN
            # windows预处理宏
            -DNOMINMAX
            -DWIN32_LEAN_AND_MEAN
            -DWINDOWS
            -DNDEBUG
            # OpenAL
            -DAL_LIBTYPE_STATIC)
elseif(LINUX)

    # 设置相关宏定义
    add_definitions(-DWEBRTC_POSIX -DWEBRTC_LINUX -DQ_OS_POSIX
            -D_GLIBCXX_USE_CXX11_ABI=1 -DOPENSSL_IS_BORINGSSL=1)
elseif(APPLE)
    # do something related to APPLE
    message(ERROR "暂不支持 Not supported temporarily")
endif()

if(UNIX)
  find_package(PkgConfig REQUIRED)

# -Wunused-parameter -pedantic -fsanitize=address,undefined,leak,integer -Wextra
# -Wall -Wmacro-redefined -Wbuiltin-macro-redefined
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -fstack-protector-all  -Wunused-function -Wstrict-overflow -Wstrict-aliasing -Wstack-protector"
  )

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
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  /D_ITERATOR_DEBUG_LEVEL=0 /_DEBUG /MTd ")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}  /D_ITERATOR_DEBUG_LEVEL=0 /DNDEBUG /MT ")
    add_definitions(-D_ITERATOR_DEBUG_LEVEL=0)

    SET (CMAKE_C_COMPILER_WORKS 1)
    SET (CMAKE_CXX_COMPILER_WORKS 1)

endif()

message(STATUS "CMAKE_C_COMPILER_ID=" ${CMAKE_C_COMPILER_ID})
message(STATUS "CMAKE_C_COMPILER=" ${CMAKE_C_COMPILER})
message(STATUS "CMAKE_C_FLAGS=" ${CMAKE_C_FLAGS})

message(STATUS "CMAKE_CXX_COMPILER_ID=" ${CMAKE_CXX_COMPILER_ID})
message(STATUS "CMAKE_CXX_COMPILER=" ${CMAKE_CXX_COMPILER})
message(STATUS "CMAKE_CXX_FLAGS=" ${CMAKE_CXX_FLAGS})
message(STATUS "CMAKE_CXX_FLAGS_DEBUG=" ${CMAKE_CXX_FLAGS_DEBUG})
message(STATUS "CMAKE_CXX_FLAGS_RELEASE=" ${CMAKE_CXX_FLAGS_RELEASE})

option(PLATFORM_EXTENSIONS "Enable platform specific extensions, requires extra dependencies" ON)