project(OkVideo)

file(GLOB ${PROJECT_NAME}_SOURCES
		*.h *.cpp
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	set(${PROJECT_NAME}_SOURCES ${${PROJECT_NAME}_SOURCES} camera/directshow.cpp camera/directshow.h)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
	set(${PROJECT_NAME}_SOURCES ${${PROJECT_NAME}_SOURCES} camera/v4l2.cpp camera/v4l2.h)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set(${PROJECT_NAME}_SOURCES ${${PROJECT_NAME}_SOURCES} camera/avfoundation.h camera/avfoundation.mm)
endif ()

pkg_check_modules(AVDEVICE REQUIRED libavdevice)
include_directories(${AVDEVICE_INCLUDE_DIRS})
pkg_check_modules(AVCODEC REQUIRED libavcodec)
include_directories(${AVCODEC_INCLUDE_DIRS})
pkg_check_modules(AVFORMAT REQUIRED libavformat)
include_directories(${AVFORMAT_INCLUDE_DIRS})
pkg_check_modules(AVUTIL REQUIRED libavutil)
include_directories(${AVUTIL_INCLUDE_DIRS})
pkg_check_modules(SWSCALE REQUIRED libswscale)
include_directories(${SWSCALE_INCLUDE_DIRS})
pkg_check_modules(SWRESAMPLE REQUIRED libswresample)
include_directories(${SWRESAMPLE_INCLUDE_DIRS})

add_library(${PROJECT_NAME} STATIC
		${${PROJECT_NAME}_SOURCES}
)

target_link_libraries(${PROJECT_NAME}
		PRIVATE ${Qt5Core_LIBRARIES}
		PRIVATE ${Qt5Gui_LIBRARIES}
		PRIVATE ${Qt5Widgets_LIBRARIES}
		PRIVATE ${AVDEVICE_LINK_LIBRARIES}
		PRIVATE ${AVCODEC_LINK_LIBRARIES}
		PRIVATE ${AVFORMAT_LINK_LIBRARIES}
		PRIVATE ${AVUTIL_LINK_LIBRARIES}
		PRIVATE ${SWSCALE_LINK_LIBRARIES}
)

if (WIN32)
	set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()
