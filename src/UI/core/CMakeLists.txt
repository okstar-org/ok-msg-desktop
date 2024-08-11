project(UICore)
set(UICore_SOURCES
    ControllerManager.cpp
    FontManager.cpp
    SettingManager.cpp
    ui.h
)

add_library(${PROJECT_NAME} STATIC
        ${UICore_SOURCES})

target_link_libraries(${PROJECT_NAME}
        PRIVATE ${Qt5Core_LIBRARIES}
)

if (WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()