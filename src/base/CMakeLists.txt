project(OkBase)


include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/plugins/include)

file(GLOB ${PROJECT_NAME}_SOURCES *.h *.cpp)

if (WIN32)
	list(APPEND ${PROJECT_NAME}_SOURCES
			system/windows/sys_info.cpp
			system/windows/autorun_win.cpp
	)
	FILE(GLOB ${PROJECT_NAME}_TEST_RESOURCES test/windows/*.cpp)
elseif (LINUX)
	list(APPEND ${PROJECT_NAME}_SOURCES
			system/linux/sys_info.cpp
			system/linux/autorun_xdg.cpp
			system/linux/x11_display.h
			system/linux/x11_display.cpp
	)
	list(APPEND PLATFORM_LIBS PRIVATE X11)
	FILE(GLOB ${PROJECT_NAME}_TEST_RESOURCES test/linux/*.cpp)
elseif (APPLE)
	list(APPEND ${PROJECT_NAME}_SOURCES
			system/macos/sys_info.cpp
			system/macos/autorun_osx.cpp)
	FILE(GLOB ${PROJECT_NAME}_TEST_RESOURCES test/macos/*.cpp)
endif ()

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES})

target_link_libraries(
		${PROJECT_NAME}
		${PLATFORM_LIBS}
		PRIVATE ${Qt5Core_LIBRARIES}
		PRIVATE ${Qt5Svg_LIBRARIES}
		PRIVATE ${Qt5Xml_LIBRARIES}
)

if (WIN32)
	set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()


# Google Test
set(${PROJECT_NAME}_TEST_TARGET "${PROJECT_NAME}Tests")
message(STATUS "Test target: ${${PROJECT_NAME}_TEST_TARGET}")

add_executable(${${PROJECT_NAME}_TEST_TARGET}
	  ${${PROJECT_NAME}_TEST_RESOURCES}
		${${PROJECT_NAME}_SOURCES}
)

# 链接gtest和gtest_main库
target_link_libraries(${${PROJECT_NAME}_TEST_TARGET}
	  PRIVATE gtest
		PRIVATE gtest_main
		${PLATFORM_LIBS}
		PRIVATE ${Qt5Core_LIBRARIES}
		PRIVATE ${Qt5Gui_LIBRARIES}
		PRIVATE ${Qt5Svg_LIBRARIES}
		PRIVATE ${Qt5Xml_LIBRARIES}
)

# 添加测试到CTest（如果你使用CTest）
add_test(NAME ${${PROJECT_NAME}_TEST_TARGET} COMMAND ${PROJECT_NAME}Tests)

