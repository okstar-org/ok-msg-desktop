project(OkMSG-Desktop)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(base)
add_subdirectory(lib)
add_subdirectory(modules)
add_subdirectory(UI)

# Add RC files.
set(${PROJECT_NAME}_RC_FILES icon/icon.rc)
# Add QRC files.
set(${PROJECT_NAME}_QRC_FILES UI/resources/resources.qrc)

set(${PROJECT_NAME}_RESOURCES ${${PROJECT_NAME}_RC_FILES}
                              ${${PROJECT_NAME}_QRC_FILES})

set(${PROJECT_NAME}_SOURCES main.cpp application.cpp launcher.cpp)

list(
  APPEND
  OK_LIBS
  UI
  IM
  IMCore
  OkRTC
  Session
  Settings
  Network
  Backend
  OkBase)

if(ENABLE_PLUGINS)
  list(APPEND OK_LIBS Plugin)
endif()

if(WIN32 AND NOT LOWER_CMAKE_BUILD_TYPE STREQUAL debug)
  add_executable(${PROJECT_NAME} WIN32 ${${PROJECT_NAME}_SOURCES}
                                       ${${PROJECT_NAME}_RESOURCES})
else()
  add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES}
                                 ${${PROJECT_NAME}_RESOURCES})
endif()

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE ${OK_LIBS}
  PRIVATE ${Qt5Core_LIBRARIES}
  PRIVATE ${Qt5Gui_LIBRARIES}
  PRIVATE ${Qt5Network_LIBRARIES}
  PRIVATE ${Qt5Widgets_LIBRARIES}
  PRIVATE ${Qt5MultimediaWidgets_LIBRARIES}
  PRIVATE ${Qt5Multimedia_LIBRARIES}
  PRIVATE ${Qt5Xml_LIBRARIES}
  PRIVATE ${Qt5Svg_LIBRARIES})

if(MSVC)
  set_property(
    TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY
                                    "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif(MSVC)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)

set(ICON_SIZES
    16x16
    24x24
    32x32
    48x48
    64x64
    72x72
    96x96
    128x128
    256x256)

foreach(SIZE ${ICON_SIZES})
  set(ICON_FILE "icon/${SIZE}.png")
  set(DESTINATION_DIR "/usr/share/icons/hicolor/${SIZE}/apps")
  set(DESTINATION_PATH "${DESTINATION_DIR}/ok-msg.png")

  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/${ICON_FILE}
    DESTINATION ${DESTINATION_DIR}
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)

  install(
    CODE "execute_process(COMMAND xdg-icon-resource install --context apps --size ${SIZE} ${DESTINATION_PATH})"
  )
endforeach()

set(DESKTOP_FILE "ok-msg.desktop")
set(DESTINATION_DIR "/usr/share/applications")

configure_file(${DESKTOP_FILE}.in ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE}
               @ONLY)

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${DESKTOP_FILE}
  DESTINATION ${DESTINATION_DIR}
  PERMISSIONS OWNER_READ GROUP_READ WORLD_READ)
