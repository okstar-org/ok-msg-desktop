project(OkSession)

include_directories(${WebRTC_DIR}/include)

find_package(ok-gloox REQUIRED)
message(STATUS "Found ok-gloox=${ok-gloox_FOUND}")
if(ok-gloox_FOUND)
    set(gloox_LIB ok-gloox::ok-gloox)
endif ()

set(${PROJECT_NAME}_SOURCES
        AccountItem.cpp
        AccountItem.h
        AuthSession.cpp
        AuthSession.h)

add_library(${PROJECT_NAME} STATIC
        ${${PROJECT_NAME}_SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE OkIMCore
    PRIVATE ${gloox_LIB}
    PUBLIC ${Qt5Core_LIBRARIES}
    PUBLIC ${Qt5Xml_LIBRARIES}
)

if (MSVC)
    set_property(
            TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif (MSVC)
