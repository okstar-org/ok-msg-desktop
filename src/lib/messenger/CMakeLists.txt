project(IMCore)

include_directories(${CMAKE_SOURCE_DIR}/3rdparty)
include_directories(${WebRTC_DIR}/include)

file(GLOB ${PROJECT_NAME}_SOURCES *.cpp)

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE ${Qt5Core_LIBRARIES}
  PRIVATE ${Qt5Xml_LIBRARIES}
  PRIVATE ${Qt5Widgets_LIBRARIES})

if(MSVC)
  set_property(
    TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY
    "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif(MSVC)
