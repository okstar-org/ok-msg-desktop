project(Session)

include_directories(${WebRTC_DIR}/include)
include_directories(${GLOOX_DIR}/include)

set(${PROJECT_NAME}_SOURCES
        AccountItem.cpp
        AccountItem.h
        AuthSession.cpp
        AuthSession.h
)

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})
target_link_libraries(${PROJECT_NAME}
        PRIVATE ${Qt5Core_LIBRARIES}

)

if (MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif (MSVC)