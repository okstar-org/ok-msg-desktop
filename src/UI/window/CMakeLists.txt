project(UIWindow)

add_definitions(-DOK_${PROJECT_NAME}_MODULE="${PROJECT_NAME}")

include_directories(${CMAKE_SOURCE_DIR}/plugins/include)

set(${PROJECT_NAME}_SOURCES widgetfactory.cpp)

add_subdirectory(login)
add_subdirectory(main)
add_subdirectory(config)

add_library(${PROJECT_NAME} STATIC
		${${PROJECT_NAME}_FORMS}
		${${PROJECT_NAME}_RESOURCES}
		${${PROJECT_NAME}_SOURCES})

target_link_libraries(${PROJECT_NAME}
		PUBLIC UIWindowLogin
		PUBLIC UIWindowMain
		PUBLIC UIWindowConfig
		PRIVATE ${Qt5Core_LIBRARIES}
		PRIVATE ${Qt5Widgets_LIBRARIES}
)

if (WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()