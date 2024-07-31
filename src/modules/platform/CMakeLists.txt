project(Platform)

file(GLOB ${PROJECT_NAME}_SOURCES src/*.h src/*.cpp)

add_library(${PROJECT_NAME} STATIC
	${${PROJECT_NAME}_SOURCES}
)

find_package(Qt5 COMPONENTS Core Widgets WebEngine WebEngineWidgets REQUIRED)

target_link_libraries(${PROJECT_NAME}
		PRIVATE ${Qt5WebEngineWidgets_LIBRARIES}
		PRIVATE ${Qt5WebEngine_LIBRARIES}
		PRIVATE ${Qt5Widgets_LIBRARIES}
		PRIVATE ${Qt5Core_LIBRARIES}
)