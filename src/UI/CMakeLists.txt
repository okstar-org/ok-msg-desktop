project(UI)

add_subdirectory(core)
add_subdirectory(window)
add_subdirectory(widget)


add_library(${PROJECT_NAME} STATIC)

target_link_libraries(${PROJECT_NAME}
        PUBLIC UICore
        PUBLIC UIWindow
        PUBLIC UIWidget
)

if (WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()