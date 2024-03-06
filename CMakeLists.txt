cmake_minimum_required(VERSION 3.21)

if(POLICY CMP0091)
  cmake_policy(SET CMP0091 NEW)
endif(POLICY CMP0091)

if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)
endif(POLICY CMP0135)

project(OkMSG-Project)

include(cmake/build.cmake)
include(cmake/develop.cmake)
include(cmake/platform.cmake)

include_directories(.)
add_subdirectory(src)

# Add RC files.
set(${PROJECT_NAME}_RC_FILES src/icon/icon.rc)

# Add QRC files.
set(${PROJECT_NAME}_QRC_FILES UI/resources/resources.qrc)

set(${PROJECT_NAME}_RESOURCES ${${PROJECT_NAME}_RC_FILES}
                              ${${PROJECT_NAME}_QRC_FILES})

set(${PROJECT_NAME}_SOURCES src/main.cpp)

qt5_add_translation(${PROJECT_NAME}_QM_FILES translations/zh_CN.ts)

file(
  WRITE "${PROJECT_BINARY_DIR}/translations.qrc.in"
  "<!DOCTYPE RCC>
<RCC version=\"1.0\">
  <qresource prefix=\"/translations/${PROJECT_NAME}\">
")

foreach(qm ${${PROJECT_NAME}_QM_FILES})
  get_filename_component(qm_name ${qm} NAME)
  file(APPEND "${PROJECT_BINARY_DIR}/translations.qrc.in"
       "    <file alias=\"${qm_name}\">${qm}</file>\n")
endforeach(qm)

file(APPEND "${PROJECT_BINARY_DIR}/translations.qrc.in" "  </qresource>
</RCC>")

execute_process(
  COMMAND
    ${CMAKE_COMMAND} -E copy_if_different
    ${PROJECT_BINARY_DIR}/translations.qrc.in
    ${PROJECT_BINARY_DIR}/translations.qrc)

qt5_add_resources(${PROJECT_NAME}_RESOURCES
                  ${PROJECT_BINARY_DIR}/translations.qrc)

if(WIN32)
  set(PLATFORM_LIBS
      swresample
      swscale
      avdevice
      avutil
      avcodec
      avfilter
      avformat
      brotlidec-static
      brotlienc-static
      charset
      exif
      fdk-aac
      # freetype
      iconv
      lzma
      mp3lame
      ogg
      openh264
      openjp2
      opus
      postproc
      qrencode
      sqlite3
      vorbis
      vorbisenc
      vorbisfile
      webp
      webpdecoder
      webpdemux
      webpmux
      x264
      x265
      sharpyuv
      OpenAL32
      vpxmt
      bz2
      d3d11
      dxgi
      shlwapi
      Iphlpapi
      dwmapi
      dmoguids
      wmcodecdspuuid
      Winmm
      amstrmid
      msdmo
      strmiids
      psapi
      Vfw32
      shell32
      oleaut32
      ole32
      user32
      advapi32
      bcrypt
      Mf
      Mfplat
      Mfreadwrite
      mfuuid
      dxva2
      evr
      strmbase)
elseif(UNIX)
  set(PLATFORM_LIBS z ssl crypto atomic)
endif()

set(ALL_LIBRARIES ${PLATFORM_LIBS})

if(ENABLE_MODULE_PAINTER)
  list(APPEND OK_LIBS Painter)
endif()

if(ENABLE_PLUGINS)
  list(APPEND OK_LIBS Plugin)
  include(${PROJECT_SOURCE_DIR}/plugins/plugins.cmake)
endif()

if(MSVC)
  set_property(
    TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY
                                    "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif(MSVC)