
# 开启插件（ON/OFF）
option(ENABLE_PLUGINS "Enable plugins" ON)
message(STATUS "Enable plugin: ${ENABLE_PLUGINS}")

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

# Enable platform module（ON/OFF）
set(ENABLE_Platform ON)
message(STATUS "Enable platform: ${ENABLE_Platform}")
if(ENABLE_Platform)
	add_definitions(-DENABLE_Platform)
endif()

# Enable meet module（ON/OFF）
set(ENABLE_Meet ON)
message(STATUS "Enable meet: ${ENABLE_Meet}")
if(ENABLE_Meet)
	add_definitions(-DENABLE_Meet)
endif()

# Enable document module（ON/OFF）
set(ENABLE_Document ON)
message(STATUS "Enable document: ${ENABLE_Document}")
if(ENABLE_Document)
	add_definitions(-DENABLE_Document)
endif()

# Enable classroom module（ON/OFF）
set(ENABLE_Classroom ON)
message(STATUS "Enable classroom: ${ENABLE_Classroom}")
if(ENABLE_Classroom)
	add_definitions(-DENABLE_Classroom)
endif()
