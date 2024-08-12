project(OkBackend)

set(${PROJECT_NAME}_Sources
        BaseService.cpp
        UserService.cpp
        PassportService.h
        PassportService.cpp
        OkCloudService.h
        OkCloudService.cpp)

add_library(${PROJECT_NAME} STATIC
        ${${PROJECT_NAME}_Sources})

target_link_libraries(${PROJECT_NAME} PRIVATE ${Qt5Core_LIBRARIES})

if (WIN32)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif ()
