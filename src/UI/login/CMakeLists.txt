project(UILoginWindow)

add_definitions(-DOK_${PROJECT_NAME}_MODULE="${PROJECT_NAME}")

file( GLOB ${PROJECT_NAME}_SOURCES src/*.cpp )
file( GLOB ${PROJECT_NAME}_UIS src/*.ui )
file(GLOB ${PROJECT_NAME}_TSS ts/*.xml)
file(GLOB ${PROJECT_NAME}_QSS qss/*.css)

qt5_wrap_ui(${PROJECT_NAME}_FORMS
       ${${PROJECT_NAME}_UIS}
)

qt5_add_translation(${PROJECT_NAME}_QM_FILES
       ${${PROJECT_NAME}_TSS}
)

set(RESOURCE_NAME "${PROJECT_BINARY_DIR}/${PROJECT_NAME}.qrc.in")

file(WRITE "${RESOURCE_NAME}"
"<!DOCTYPE RCC>
<RCC version=\"1.0\">
")

# ts
file(APPEND "${RESOURCE_NAME}"
        "<qresource prefix=\"/translations/${PROJECT_NAME}\">
")
foreach (qm ${${PROJECT_NAME}_QM_FILES})
    get_filename_component(qm_name ${qm} NAME)
    file(APPEND "${RESOURCE_NAME}"
        "    <file alias=\"${qm_name}\">${qm}</file>\n")
endforeach (qm)
file(APPEND "${RESOURCE_NAME}"
"</qresource>")

# qss
file(APPEND "${RESOURCE_NAME}"
"
<qresource prefix=\"/qss\">
")
foreach (file ${${PROJECT_NAME}_QSS})
    get_filename_component(file_name ${file} NAME)
    file(APPEND "${RESOURCE_NAME}"
            "    <file alias=\"${file_name}\">${file}</file>\n")
endforeach (file)
file(APPEND "${RESOURCE_NAME}"
        "</qresource>")

file(APPEND "${RESOURCE_NAME}" "</RCC>")

execute_process(COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${RESOURCE_NAME}
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.qrc)

qt5_add_resources(
    ${PROJECT_NAME}_RESOURCES
    ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.qrc
)

add_library(${PROJECT_NAME} STATIC
    ${${PROJECT_NAME}_FORMS}
    ${${PROJECT_NAME}_RESOURCES}
    ${${PROJECT_NAME}_QM_FILES}
    ${${PROJECT_NAME}_SOURCES})

target_link_libraries(${PROJECT_NAME}
        PRIVATE OkSession
        PRIVATE ${Qt5Widgets_LIBRARIES}
        PRIVATE ${Qt5Svg_LIBRARIES}

)

if (WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()
