project(OkSession)

include_directories(${WebRTC_DIR}/include)

set(${PROJECT_NAME}_SOURCES
        AccountItem.cpp
        AccountItem.h
        AuthSession.cpp
        AuthSession.h)

add_library(${PROJECT_NAME} STATIC
        ${${PROJECT_NAME}_SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE OkIMCore
    PUBLIC ${Qt5Core_LIBRARIES}
    PUBLIC ${Qt5Xml_LIBRARIES}
)

if (WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()
