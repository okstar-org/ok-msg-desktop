project(IM)
add_definitions(-DOK_${PROJECT_NAME}_MODULE="${PROJECT_NAME}")

include_directories(${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})



set(CMAKE_AUTOUIC_SEARCH_PATHS ./src src/widget src/widget/form)

file(GLOB_RECURSE ${PROJECT_NAME}_FORMS src/*.ui)
qt5_wrap_ui(${${PROJECT_NAME}_FORMS})

file(GLOB_RECURSE ${PROJECT_NAME}_SOURCES
		src/*.h src/*.cpp
)

list(FILTER ${PROJECT_NAME}_SOURCES EXCLUDE REGEX "src/platform/")

if (UNIX)
	set(${PROJECT_NAME}_SOURCES ${${PROJECT_NAME}_SOURCES}
			src/platform/posixsignalnotifier.cpp
			src/platform/posixsignalnotifier.h)
endif ()

if (PLATFORM_EXTENSIONS)
	set(${PROJECT_NAME}_SOURCES ${${PROJECT_NAME}_SOURCES}
			src/platform/capslock.h
			src/platform/timer.h)
	if (WIN32)
		set(${PROJECT_NAME}_SOURCES ${${PROJECT_NAME}_SOURCES}
				src/platform/capslock_win.cpp
				src/platform/timer_win.cpp)
	elseif (${X11_EXT})
		set(${PROJECT_NAME}_SOURCES ${${PROJECT_NAME}_SOURCES}
				src/platform/capslock_x11.cpp
				src/platform/timer_x11.cpp)
	elseif (${APPLE_EXT})
		set(${PROJECT_NAME}_SOURCES ${${PROJECT_NAME}_SOURCES}
				src/platform/capslock_osx.cpp
				src/platform/timer_osx.cpp
		)
	endif ()
endif ()

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
  <qresource prefix=\"/translations/${PROJECT_NAME}\">
")

foreach (qm ${${PROJECT_NAME}_QM_FILES})
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

set(SMILEY_RESOURCES
		${PROJECT_SOURCE_DIR}/smileys/emojione.qrc
		${PROJECT_SOURCE_DIR}/smileys/smileys.qrc)

qt5_add_resources(
		${PROJECT_NAME}_RESOURCES
		res.qrc
		${PROJECT_BINARY_DIR}/${PROJECT_NAME}.qrc
		${SMILEY_RESOURCES}
		# DEPENDS ${${PROJECT_NAME}_QM_FILES} # OPTIONS ${RCC_OPTIONS}
)

add_library(${PROJECT_NAME} STATIC
		${${PROJECT_NAME}_FORMS}
		${${PROJECT_NAME}_SOURCES}
		${${PROJECT_NAME}_QM_FILES}
		${${PROJECT_NAME}_RESOURCES}
)


target_link_libraries(${PROJECT_NAME}
		PRIVATE OkMessenger
		PRIVATE OkStorage
		PRIVATE OkAudio
		PRIVATE OkVideo
		PRIVATE ${ok-gloox_LIBRARIES}
		PUBLIC ${Qt5Core_LIBRARIES}
		PUBLIC ${Qt5Gui_LIBRARIES}
		PUBLIC ${Qt5Widgets_LIBRARIES}
		PUBLIC ${Qt5Network_LIBRARIES}
		PUBLIC ${Qt5Xml_LIBRARIES}
		PUBLIC ${Qt5Svg_LIBRARIES}
		PUBLIC ${CMAKE_REQUIRED_LIBRARIES}	
		PUBLIC range-v3::meta range-v3::concepts range-v3::range-v3
)

if (WIN32)
	set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()

