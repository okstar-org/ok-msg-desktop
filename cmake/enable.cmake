
# 开启插件（ON/OFF）
option(ENABLE_PLUGINS "Enable plugins" ON)
message(STATUS "ENABLE_PLUGINS=${ENABLE_PLUGINS}")

if(ENABLE_PLUGINS)
  add_definitions(-DOK_PLUGIN)
  include(${PROJECT_SOURCE_DIR}/plugins/plugins.cmake)

  if(NOT DEV_MODE)
    set(LIB_SUFFIX
            ""
            CACHE STRING "Define suffix of directory name")
    set(OK_LIBDIR
            "${CMAKE_INSTALL_PREFIX}/lib${LIB_SUFFIX}/${PROJECT_NAME}"
            CACHE STRING "${PROJECT_NAME} libraries directory")
    set(OK_DATADIR
            "${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME}"
            CACHE STRING "${PROJECT_NAME} data directory")
  else()
    set(OK_LIBDIR ".")
    set(OK_DATADIR ".")
  endif()
endif()


# Enable classroom module（ON/OFF）
option(ENABLE-Classroom "Enable Classroom module" ON)
# Enable document module
option(ENABLE-Document "Enable Document module" ON)
# Enable meet module
option(ENABLE-Meet "Enable Meet module" ON)
