project(UIComponent)
file( GLOB ${PROJECT_NAME}_SRCS *.cpp )
add_library( ${PROJECT_NAME} ${${PROJECT_NAME}_SRCS} )

target_link_libraries(${PROJECT_NAME}
        PRIVATE ${Qt5Core_LIBRARIES}
        PRIVATE ${Qt5Widgets_LIBRARIES}
        PRIVATE ${Qt5Xml_LIBRARIES}
)