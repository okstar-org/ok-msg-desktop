project(OkSettings)



set(${PROJECT_NAME}_SOURCES
        translator.h
        translator.cpp
        applicationinfo.h
        applicationinfo.cpp
)

add_library(${PROJECT_NAME} STATIC
        ${${PROJECT_NAME}_SOURCES})

target_link_libraries(${PROJECT_NAME}
        PRIVATE ${Qt5Core_LIBRARIES}
        PRIVATE ${Qt5Widgets_LIBRARIES}
)

if(MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif (MSVC)