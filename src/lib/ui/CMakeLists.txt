project(OkUI)

file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES *.h *.cpp)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES})

pkg_check_modules(QRENCODE REQUIRED libqrencode)
include_directories(${QRENCODE_INCLUDE_DIRS})

find_package(Qt5 COMPONENTS Core Widgets WebChannel WebSockets WebEngine WebEngineWidgets REQUIRED)

target_link_libraries(${PROJECT_NAME}
		PRIVATE ${Qt5WebEngineWidgets_LIBRARIES}
		PRIVATE ${Qt5WebEngine_LIBRARIES}
		PRIVATE ${Qt5WebChannel_LIBRARIES}
		PRIVATE ${Qt5WebSockets_LIBRARIES}
		PRIVATE ${Qt5Widgets_LIBRARIES}
		PRIVATE ${Qt5Gui_LIBRARIES}
		PRIVATE ${Qt5Core_LIBRARIES}
		PRIVATE ${QRENCODE_LINK_LIBRARIES}
)

if (WIN32)
	set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()
