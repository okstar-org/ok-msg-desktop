project(OkBase)

include_directories(../plugins/include)
include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/plugins/include)

set(${PROJECT_NAME}_SOURCES
    files.cpp
    logs.cpp
    StringUtils.cpp
    timer.cpp
    times.cpp
    task.h
    utils.cpp
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
    PageFactory.h)

if(WIN32)
  set(${PROJECT_NAME}_SOURCES
      ${${PROJECT_NAME}_SOURCES} system/windows/sys_info.cpp
      system/windows/autorun_win.cpp)
elseif(LINUX)
  set(${PROJECT_NAME}_SOURCES
      ${${PROJECT_NAME}_SOURCES} system/linux/sys_info.cpp
      system/linux/autorun_xdg.cpp)
endif()

add_library(${PROJECT_NAME} STATIC
        ${${PROJECT_NAME}_SOURCES})

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE ${Qt5Core_LIBRARIES}
  PRIVATE ${Qt5Gui_LIBRARIES}
  PRIVATE ${Qt5Svg_LIBRARIES}
  PRIVATE ${Qt5Xml_LIBRARIES})

if(MSVC)
  set_property(
    TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY
                                    "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif(MSVC)
