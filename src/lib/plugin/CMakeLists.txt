project(Plugin)

include_directories(../../base)
include_directories(iconset/)

message(STATUS "Plugin header: ${CMAKE_SOURCE_DIR}/plugins/include")
include_directories(${CMAKE_SOURCE_DIR}/plugins/include)

set(${PROJECT_NAME}_SOURCES
		iconset/anim.h
		iconset/anim.cpp
		iconset/iconset.h
		iconset/iconset.cpp
		varianttree.h
		varianttree.cpp
		optionstree.h
		optionstree.cpp
		OkOptions.h
		OkOptions.cpp
		pluginhost.h
		pluginhost.cpp
		pluginmanager.h
		pluginmanager.cpp
		PluginInfo.h
		PluginInfo.cpp
		textutil.h
		textutil.cpp
		coloropt.h
		coloropt.cpp
		common.h
		common.cpp)

add_library(${PROJECT_NAME} STATIC
		${${PROJECT_NAME}_SOURCES})

target_link_libraries(${PROJECT_NAME}
		PRIVATE OkBase
		PRIVATE OkSettings
		PRIVATE ${Qt5Core_LIBRARIES}
		PRIVATE ${Qt5Xml_LIBRARIES}
		PRIVATE ${Qt5Widgets_LIBRARIES}
		PRIVATE ${Qt5Svg_LIBRARIES}
		PRIVATE ${Qt5Multimedia_LIBRARIES})

if (WIN32)
	set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()
