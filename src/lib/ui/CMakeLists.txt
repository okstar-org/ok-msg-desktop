project(OkUI)

file(GLOB ${PROJECT_NAME}_SOURCES
		*.h *.cpp
		platform/*.h platform/*.cpp
		widget/*.ui
		widget/*.h widget/*.cpp
		widget/tools/*.h widget/tools/*.cpp
		layout/*.h layout/*.cpp
		utils/*.h utils/*.cpp
)


add_library(${PROJECT_NAME} STATIC
		${${PROJECT_NAME}_SOURCES}
)

target_link_libraries(${PROJECT_NAME}
		PRIVATE ${Qt5Core_LIBRARIES}
		PRIVATE ${Qt5Widgets_LIBRARIES}
		PRIVATE ${Qt5Gui_LIBRARIES}
)

if (WIN32)
	set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()
