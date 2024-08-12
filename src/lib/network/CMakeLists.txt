project(OkNetwork)

set(${PROJECT_NAME}_SOURCES
    ImageLoader.h
    ImageLoader.cpp
    NetworkHttp.h
    NetworkHttp.cpp)

add_library(${PROJECT_NAME} STATIC
		${${PROJECT_NAME}_SOURCES})

target_link_libraries(${PROJECT_NAME}
		PRIVATE ${Qt5Core_LIBRARIES}
		PRIVATE ${Qt5Network_LIBRARIES})

if (WIN32)
	set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY	"MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()
