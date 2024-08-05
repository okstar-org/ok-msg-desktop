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
    add_definitions(-DQ_OS_POSIX)
elseif(APPLE)
    # do something related to APPLE
    add_definitions(-DAPPLE)
endif()


option(PLATFORM_EXTENSIONS "Enable platform specific extensions, requires extra dependencies" ON)
