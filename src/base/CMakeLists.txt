project(OkBase)

include_directories(../plugins/include)
include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/plugins/include)

set(${PROJECT_NAME}_SOURCES
    basic_types.h

    files.h
    logs.cpp
    StringUtils.cpp
    timer.cpp
    times.h
    task.h
    widgets.h
    system/sys_info.h
    jid.h
    jid.cpp
    OkAccount.h
    OkAccount.cpp
    svgiconengine.h
    svgiconengine.cpp
    hashs.h
    hashs.cpp
    images.h
    xmls.h
    jsons.h
    r.h
    Page.h
    Page.cpp
    PageFactory.h
    OkSettings.h
    OkSettings.cpp
    OkProcess.h
    utils.h
    SvgUtils.h
    RecursiveSignalBlocker.h
    RecursiveSignalBlocker.cpp
		uuid.h
    stylediconbutton.h
    stylediconbutton.cpp
)

if(WIN32)
  set(${PROJECT_NAME}_SOURCES
      ${${PROJECT_NAME}_SOURCES} system/windows/sys_info.cpp
      system/windows/autorun_win.cpp)
elseif(LINUX)
  set(${PROJECT_NAME}_SOURCES
      ${${PROJECT_NAME}_SOURCES} system/linux/sys_info.cpp
      system/linux/autorun_xdg.cpp)
elseif(APPLE)
  set(${PROJECT_NAME}_SOURCES
      ${${PROJECT_NAME}_SOURCES} system/macos/sys_info.cpp
      system/macos/autorun_osx.cpp)
endif()

add_library(${PROJECT_NAME} STATIC
        ${${PROJECT_NAME}_SOURCES})

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE ${Qt5Core_LIBRARIES}
  PRIVATE ${Qt5Gui_LIBRARIES}
  PRIVATE ${Qt5Svg_LIBRARIES}
  PRIVATE ${Qt5Xml_LIBRARIES})

if(WIN32)
  set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
