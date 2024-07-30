project(UIWidget)

set(${PROJECT_NAME}_RESOUCRES MoveableBar.cpp GenericForm.h GenericForm.cpp)

add_library(${PROJECT_NAME} STATIC
        ${${PROJECT_NAME}_RESOUCRES})

target_link_libraries(${PROJECT_NAME}
        PRIVATE ${Qt5Core_LIBRARIES}
        PRIVATE ${Qt5Widgets_LIBRARIES}
)

if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif (MSVC)