project(Platform)
add_definitions(-DOK_${PROJECT_NAME}_MODULE="${PROJECT_NAME}")

file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES src/*.h src/*.cpp)
file(GLOB_RECURSE ${PROJECT_NAME}_RESOURCES res/*.html res/*.js)
file(GLOB_RECURSE ${PROJECT_NAME}_UIS src/*.ui)

file(GLOB ${PROJECT_NAME}_TSS
		ts/*.ts
		ts/*.xml
)
qt5_add_translation(
		${PROJECT_NAME}_QM_FILES
		${${PROJECT_NAME}_TSS}
)

set(RESOURCE_NAME "${PROJECT_BINARY_DIR}/${PROJECT_NAME}.qrc.in")

file(WRITE "${RESOURCE_NAME}"
		"<!DOCTYPE RCC>
<RCC version=\"1.0\">

")
# ts
file(APPEND "${RESOURCE_NAME}" "<qresource prefix=\"/translations/${PROJECT_NAME}\">
") #换行
foreach (qm ${${PROJECT_NAME}_QM_FILES})
	get_filename_component(qm_name ${qm} NAME)
	file(APPEND "${RESOURCE_NAME}"
			#留出格式空间
			"    <file alias=\"${qm_name}\">${qm}</file>\n")
endforeach (qm)
file(APPEND "${RESOURCE_NAME}"
		"</qresource>")

file(APPEND "${RESOURCE_NAME}" "
<qresource prefix=\"/res/${PROJECT_NAME}\">
") #换行

foreach (f ${${PROJECT_NAME}_RESOURCES})
	get_filename_component(f_name ${f} NAME)
	file(APPEND "${RESOURCE_NAME}"
			"    <file alias=\"${f_name}\">${f}</file>\n")
endforeach (f)

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

if(WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()