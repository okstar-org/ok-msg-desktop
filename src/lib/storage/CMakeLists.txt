project(OkStorage)

file(GLOB ${PROJECT_NAME}_SOURCES
		*.h *.cpp
		log/*.h log/*.cpp
		settings/*.h settings/*.cpp
		db/*.h db/*.cpp
		cache/*.h cache/*.cpp
)


pkg_check_modules(sqlite3 REQUIRED sqlite3)
include_directories(${sqlite3_INCLUDE_DIRS})

add_library(${PROJECT_NAME} STATIC
		${${PROJECT_NAME}_SOURCES}
)

target_link_libraries(${PROJECT_NAME}
		PRIVATE ${Qt5Core_LIBRARIES}
		PRIVATE ${Qt5Gui_LIBRARIES}
		PRIVATE ${sqlite3_LINK_LIBRARIES}
)

if (WIN32)
	set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()
