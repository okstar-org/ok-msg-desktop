project(OkIMCore)


set(CMAKE_CXX_STANDARD 20)

if (WIN32)
	add_definitions(-D_WIN32 -DWIN32 -D_WINDOWS -D_UNICODE)
endif()

find_package(ok-gloox REQUIRED)
if(ok-gloox_FOUND)
	search_dependency(ok-gloox PACKAGE ok-gloox)
	set(gloox_LIB ok-gloox)
endif ()

add_definitions(
		-DLOG_XMPP # xmpp logs
		-DWANT_PING # xmpp ping
		-DENABLE_SEND_RAW_XML # send raw xmls
		-DLOG_TO_FILE
		-DINBANDBYTESTREAM_TEST
		-DCLIENTBASE_TEST)

# ==== Gloox ====

include_directories(${WebRTC_DIR}/include)

file(GLOB ${PROJECT_NAME}_SOURCES *.cpp *.h)

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE ${ok-gloox_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME}
		PUBLIC Plugin
		PRIVATE OkRTC
		PRIVATE ${gloox_LIB}
		PRIVATE ${Qt5Core_LIBRARIES}
		PRIVATE ${Qt5Xml_LIBRARIES}
		PRIVATE ${Qt5Widgets_LIBRARIES})


if (WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()
