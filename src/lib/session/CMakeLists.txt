project(OkSession)

include_directories(${gloox_SOURCE_DIR}/src)
include_directories(${WebRTC_DIR}/include)

set(${PROJECT_NAME}_SOURCES
        AccountItem.cpp
        AccountItem.h
        AuthSession.cpp
        AuthSession.h)

add_library(${PROJECT_NAME} STATIC
        ${${PROJECT_NAME}_SOURCES})

target_link_libraries(${PROJECT_NAME}
    PUBLIC ${Qt5Core_LIBRARIES}
    PUBLIC ${Qt5Xml_LIBRARIES}
)

if (MSVC)
    set_property(
            TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif (MSVC)
