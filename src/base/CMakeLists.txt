project(OkBase)

include(FetchContent)

# 设置googletest的版本（可选）
set(GOOGLETEST_VERSION "release-1.11.0") # 替换为你想要的版本
FetchContent_Declare(
		googletest
		GIT_REPOSITORY https://github.com/google/googletest.git
		GIT_TAG ${GOOGLETEST_VERSION} # 如果设置了版本
)

# 为了简化，我们不将gtest安装到系统路径中，而是直接在构建目录中构建和使用它
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
# 启用测试
enable_testing()


include_directories(../plugins/include)
include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/plugins/include)

set(${PROJECT_NAME}_SOURCES
		basic_types.h

		files.h
		logs.cpp
		StringUtils.cpp
		timer.cpp
		times.h
		task.h
		widgets.h
		system/sys_info.h
		jid.h
		jid.cpp
		OkAccount.h
		OkAccount.cpp
		svgiconengine.h
		svgiconengine.cpp
		hashs.h
		hashs.cpp
		images.h
		xmls.h
		jsons.h
		r.h
		Page.h
		Page.cpp
		PageFactory.h
		OkProcess.h
		utils.h
		SvgUtils.h
		RecursiveSignalBlocker.h
		RecursiveSignalBlocker.cpp
		uuid.h
		stylediconbutton.h
		stylediconbutton.cpp
		roundedmaskcontainer.h
		roundedmaskcontainer.cpp
		verticalonlyscroller.h
		verticalonlyscroller.cpp
		MessageBox.h
		MessageBox.cpp
		shadowbackground.h
		shadowbackground.cpp
		RoundedPixmapLabel.h
		RoundedPixmapLabel.cpp
		Styles.h
		identicon.cpp
		identicon.h
)


if (WIN32)
	set(${PROJECT_NAME}_SOURCES
			${${PROJECT_NAME}_SOURCES} system/windows/sys_info.cpp
			system/windows/autorun_win.cpp)
elseif (LINUX)
	set(${PROJECT_NAME}_SOURCES ${${PROJECT_NAME}_SOURCES}
			system/linux/sys_info.cpp
			system/linux/autorun_xdg.cpp
			system/linux/x11_display.h
			system/linux/x11_display.cpp
	)
	list(APPEND PLATFORM_LIBS PRIVATE X11)
elseif (APPLE)
	set(${PROJECT_NAME}_SOURCES
			${${PROJECT_NAME}_SOURCES} system/macos/sys_info.cpp
			system/macos/autorun_osx.cpp)
endif ()

add_library(${PROJECT_NAME} STATIC
		${${PROJECT_NAME}_SOURCES})

target_link_libraries(
		${PROJECT_NAME}
		${PLATFORM_LIBS}
		PRIVATE ${Qt5Core_LIBRARIES}
		PRIVATE ${Qt5Gui_LIBRARIES}
		PRIVATE ${Qt5Svg_LIBRARIES}
		PRIVATE ${Qt5Xml_LIBRARIES}
)

if (WIN32)
	set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()


FILE(GLOB ${PROJECT_NAME}_TEST_RESOURCES test/*.cpp)
message(STATUS "Test files: ${${PROJECT_NAME}_TEST_RESOURCES}")

add_executable(runTests
		${${PROJECT_NAME}_TEST_RESOURCES}
		${${PROJECT_NAME}_SOURCES}
)

# 链接gtest和gtest_main库
target_link_libraries(runTests
		PRIVATE X11
		PRIVATE gtest
		PRIVATE gtest_main
		PRIVATE ${Qt5Core_LIBRARIES}
		PRIVATE ${Qt5Gui_LIBRARIES}
		PRIVATE ${Qt5Svg_LIBRARIES}
		PRIVATE ${Qt5Xml_LIBRARIES}
)

# 添加测试到CTest（如果你使用CTest）
add_test(NAME RunAllTests COMMAND runTests)