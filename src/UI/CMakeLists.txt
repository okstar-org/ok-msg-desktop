project(UI)

add_definitions(-DOK_${PROJECT_NAME}_MODULE="${PROJECT_NAME}")

include_directories(${CMAKE_SOURCE_DIR}/plugins/include)

set(${PROJECT_NAME}_SOURCES)

add_subdirectory(login)
add_subdirectory(main)

add_library(${PROJECT_NAME} STATIC
		${${PROJECT_NAME}_FORMS}
		${${PROJECT_NAME}_RESOURCES}
		${${PROJECT_NAME}_SOURCES})

target_link_libraries(${PROJECT_NAME}
		PUBLIC UILoginWindow
		PUBLIC UIMainWindow
		PUBLIC Config
		PRIVATE ${Qt5Core_LIBRARIES}
		PRIVATE ${Qt5Widgets_LIBRARIES}
)

if (WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()