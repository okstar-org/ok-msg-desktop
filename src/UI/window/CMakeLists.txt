project(UIWindow)

add_definitions(-DOK_${PROJECT_NAME}_MODULE="${PROJECT_NAME}")

include_directories(${CMAKE_SOURCE_DIR}/plugins/include)

set(${PROJECT_NAME}_SOURCES
        BaseWindow.cpp
        widgetfactory.cpp
        WindowManager.h
        WindowManager.cpp
)

add_subdirectory(login)
add_subdirectory(main)
add_subdirectory(config)

add_library(${PROJECT_NAME}
    ${${PROJECT_NAME}_FORMS}
    ${${PROJECT_NAME}_RESOURCES}
    ${${PROJECT_NAME}_SOURCES})

target_link_libraries(${PROJECT_NAME}
        PRIVATE ${Qt5Core_LIBRARIES}
        PRIVATE ${Qt5Widgets_LIBRARIES}

        UIWindowLogin
        UIWindowMain
        UIWindowConfig
)

if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif (MSVC)
