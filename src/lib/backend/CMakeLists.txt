project(Backend)

set(${PROJECT_NAME}_Sources
    BaseService.cpp UserService.cpp PassportService.h PassportService.cpp
    OkCloudService.h OkCloudService.cpp)

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_Sources})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Qt5Core_LIBRARIES})
if(MSVC)
  set_property(
    TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY
                                    "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif(MSVC)
