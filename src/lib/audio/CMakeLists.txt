project(OkAudio)

file(GLOB ${PROJECT_NAME}_SOURCES
		*.h *.cpp
		backend/*.h backend/*.cpp
)

pkg_check_modules(OpenAL REQUIRED openal)
include_directories(${OpenAL_INCLUDE_DIRS})


add_library(${PROJECT_NAME} STATIC
            ${${PROJECT_NAME}_SOURCES}
)

target_link_libraries(${PROJECT_NAME}
		PRIVATE ${Qt5Core_LIBRARIES}
        PRIVATE ${Qt5Multimedia_LIBRARIES}
		PRIVATE ${OpenAL_LINK_LIBRARIES}
)

if (WIN32)
	set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()
