project(UI)

add_subdirectory(core)
add_subdirectory(window)
add_subdirectory(widget)
add_subdirectory(component)

add_library(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
        PUBLIC UICore
        PUBLIC UIWindow
        PUBLIC UIWidget
        PUBLIC UIComponent)

if (MSVC)
    set_property(
            TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif (MSVC)
