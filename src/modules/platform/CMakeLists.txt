project(Platform)

file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES src/*.h src/*.cpp)
file(GLOB_RECURSE ${PROJECT_NAME}_RESOURCES res/*.html res/*.js)
file(GLOB_RECURSE ${PROJECT_NAME}_UIS src/*.ui)


set(RESOURCE_NAME "${PROJECT_BINARY_DIR}/${PROJECT_NAME}.qrc.in")

file(WRITE "${RESOURCE_NAME}"
		"<!DOCTYPE RCC>
<RCC version=\"1.0\">
  <qresource prefix=\"/res/${PROJECT_NAME}\">
")

foreach (qm ${${PROJECT_NAME}_RESOURCES})
	get_filename_component(qm_name ${qm} NAME)
	file(APPEND "${RESOURCE_NAME}"
			"    <file alias=\"${qm_name}\">${qm}</file>\n")
endforeach (qm)

file(APPEND "${RESOURCE_NAME}"
		"</qresource>
</RCC>
")

execute_process(
		COMMAND
		${CMAKE_COMMAND} -E copy_if_different
		${RESOURCE_NAME}
		${PROJECT_BINARY_DIR}/${PROJECT_NAME}.qrc)

qt5_add_resources(
		${PROJECT_NAME}_RESOURCES
		${PROJECT_BINARY_DIR}/${PROJECT_NAME}.qrc
)

qt5_wrap_ui(${PROJECT_NAME}_FORMS
		${${PROJECT_NAME}_UIS}
)

add_library(${PROJECT_NAME} STATIC
	${${PROJECT_NAME}_SOURCES}
	${${PROJECT_NAME}_RESOURCES}
		${${PROJECT_NAME}_FORMS}
)

find_package(Qt5 COMPONENTS Core Widgets WebChannel WebSockets WebEngine WebEngineWidgets REQUIRED)

target_link_libraries(${PROJECT_NAME}
		PRIVATE ${Qt5WebChannel_LIBRARIES}
		PRIVATE ${Qt5WebSockets_LIBRARIES}
		PRIVATE ${Qt5WebEngineWidgets_LIBRARIES}
		PRIVATE ${Qt5WebEngine_LIBRARIES}
		PRIVATE ${Qt5Widgets_LIBRARIES}
		PRIVATE ${Qt5Core_LIBRARIES}
)
