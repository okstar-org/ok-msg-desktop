project(OkBoard)

file(GLOB ${PROJECT_NAME}_SOURCES
		*.h *.cpp
)

find_package(ok-gloox REQUIRED)
if (ok-gloox_FOUND)
	set(gloox_LIB ok-gloox)
endif ()

add_library(${PROJECT_NAME} STATIC
		${${PROJECT_NAME}_SOURCES}
)

target_link_libraries(${PROJECT_NAME}
		PRIVATE OkMessenger
		PRIVATE ${gloox_LIB}
		PRIVATE ${Qt5Core_LIBRARIES}
)

if (MSVC)
	set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif (MSVC)