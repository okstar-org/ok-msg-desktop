add_subdirectory(im)
add_subdirectory(platform)
add_subdirectory(config)

if(ENABLE_Meet)
	add_subdirectory(meet)
endif()

if(ENABLE_Document)
	add_subdirectory(document)
endif()

if(ENABLE_Classroom)
	add_subdirectory(classroom)
endif()
