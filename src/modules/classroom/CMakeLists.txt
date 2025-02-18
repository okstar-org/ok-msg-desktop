project(Classroom)
add_definitions(-DOK_${PROJECT_NAME}_MODULE="${PROJECT_NAME}")


include_directories(.)
include_directories(./src)
include_directories(./src/component)

file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES src/*.h src/*.cpp)
file(GLOB_RECURSE ${PROJECT_NAME}_RESOURCES res/*.html res/*.js)
file(GLOB_RECURSE ${PROJECT_NAME}_UIS src/*.ui)

file(GLOB ${PROJECT_NAME}_TSS
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
		res/${PROJECT_NAME}Res.qrc
)

qt5_wrap_ui(${PROJECT_NAME}_FORMS
		${${PROJECT_NAME}_UIS}
)


add_library(${PROJECT_NAME} STATIC
		${${PROJECT_NAME}_SOURCES}
		${${PROJECT_NAME}_RESOURCES}
		${${PROJECT_NAME}_FORMS}
)

list (APPEND ${PROJECT_NAME}_LIBS
   PRIVATE OkBoard
   PRIVATE OkBase
   PRIVATE OkMessenger
)

target_link_libraries(${PROJECT_NAME}
                PRIVATE ${${PROJECT_NAME}_LIBS}
		PRIVATE ${Qt5Widgets_LIBRARIES}
		PRIVATE ${Qt5Core_LIBRARIES}
)

if (MSVC)
	set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()


# Google Test
FILE(GLOB ${PROJECT_NAME}_TEST_RESOURCES test/*.cpp)
message(STATUS "Test files: ${${PROJECT_NAME}_TEST_RESOURCES}")

add_executable(${PROJECT_NAME}Tests
                ${${PROJECT_NAME}_TEST_RESOURCES}
)

# 链接gtest和gtest_main库
target_link_libraries(${PROJECT_NAME}Tests
                PRIVATE gtest
                PRIVATE gtest_main
                PRIVATE ${Qt5Core_LIBRARIES}
                PRIVATE ${Qt5Gui_LIBRARIES}
                PRIVATE ${Qt5Svg_LIBRARIES}
                PRIVATE ${Qt5Xml_LIBRARIES}
)


# 添加测试到CTest（如果你使用CTest）
add_test(NAME Run${PROJECT_NAME}Tests COMMAND ${PROJECT_NAME}Tests)

