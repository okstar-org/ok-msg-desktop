project(OkSession)

include_directories(${WebRTC_DIR}/include)

file(GLOB ${PROJECT_NAME}_SOURCES
		*.h *.cpp
)

add_library(${PROJECT_NAME} STATIC
		${${PROJECT_NAME}_SOURCES})

target_link_libraries(${PROJECT_NAME}
		PRIVATE OkIMCore
		PUBLIC ${Qt5Core_LIBRARIES}
		PUBLIC ${Qt5Xml_LIBRARIES}
)

if (WIN32)
	set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()
