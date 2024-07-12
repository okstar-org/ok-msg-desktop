project(IM)
add_definitions(-DOK_${PROJECT_NAME}_MODULE="${PROJECT_NAME}")

if (WIN32)
    find_package(PkgConfig REQUIRED)
   

    pkg_check_modules(AVDEVICE REQUIRED libavdevice)
    pkg_check_modules(AVCODEC REQUIRED libavcodec)
    pkg_check_modules(AVFORMAT REQUIRED libavformat)
    pkg_check_modules(AVUTIL REQUIRED libavutil)
    pkg_check_modules(SWSCALE REQUIRED libswscale)
    pkg_check_modules(SWRESAMPLE REQUIRED libswresample)

    pkg_check_modules(QRENCODE REQUIRED libqrencode )
    pkg_check_modules(OpenAL REQUIRED openal)
    pkg_check_modules(sqlite3 REQUIRED sqlite3)
    pkg_check_modules(exif REQUIRED libexif)

    pkg_check_modules(JPEG REQUIRED libjpeg)

    list(APPEND ALL_LIBRARIES
            ${AVDEVICE_LINK_LIBRARIES}
            ${AVCODEC_LINK_LIBRARIES}
            ${AVFORMAT_LINK_LIBRARIES}
            ${AVUTIL_LINK_LIBRARIES}
            ${SWSCALE_LINK_LIBRARIES}
            ${QRENCODE_LINK_LIBRARIES}
            ${JPEG_LINK_LIBRARIES}
            PRIVATE ${exif_LINK_LIBRARIES}
            PRIVATE ${sqlite3_LINK_LIBRARIES}
            PRIVATE ${OpenAL_LINK_LIBRARIES}
    )
elseif (UNIX)
    include(./cmake/Dependencies.cmake)
endif ()


include_directories(${PROJECT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_AUTOUIC_SEARCH_PATHS ./src src/widget src/widget/form)

qt5_wrap_ui(
        ${PROJECT_NAME}_FORMS
        src/mainwindow.ui
        src/chatlog/content/filetransferwidget.ui
        src/widget/ChatWidget.ui
        src/widget/ContactWidget.ui
        src/widget/form/aboutfriendform.ui
        src/widget/form/aboutgroupform.ui
        src/widget/form/addfriendform.ui
        src/widget/form/loadhistorydialog.ui
        src/widget/form/profileform.ui
        src/widget/tool/removefrienddialog.ui
        src/widget/form/searchsettingsform.ui
        src/widget/form/setpassworddialog.ui
        src/widget/form/settings/aboutsettings.ui
        src/widget/form/settings/advancedsettings.ui
        src/widget/form/settings/avform.ui
        src/widget/form/settings/generalsettings.ui
        src/widget/form/settings/privacysettings.ui
        src/widget/form/settings/userinterfacesettings.ui
    src/widget/form/CallDurationForm.ui
)

set(${PROJECT_NAME}_SOURCES
        src/audio/audio.cpp
        src/audio/audio.h
        src/audio/backend/alsink.cpp
        src/audio/backend/alsink.h
        src/audio/backend/alsource.cpp
        src/audio/backend/alsource.h
        src/audio/backend/openal.cpp
        src/audio/backend/openal.h
        src/audio/iaudiosettings.h
        src/audio/iaudiocontrol.h
        src/audio/iaudiosink.h
        src/audio/iaudiosource.h
        src/chatlog/chatlinecontent.cpp
        src/chatlog/chatlinecontent.h
        src/chatlog/chatlinecontentproxy.cpp
        src/chatlog/chatlinecontentproxy.h
        src/chatlog/chatline.cpp
        src/chatlog/chatline.h
        src/chatlog/chatmessageitem.cpp
        src/chatlog/chatmessageitem.h
        src/chatlog/chatlog.cpp
        src/chatlog/chatlog.h
        src/chatlog/chatmessage.cpp
        src/chatlog/chatmessage.h
        src/chatlog/content/filetransferwidget.cpp
        src/chatlog/content/filetransferwidget.h
        src/chatlog/content/image.cpp
        src/chatlog/content/image.h
        src/chatlog/content/notificationicon.cpp
        src/chatlog/content/notificationicon.h
        src/chatlog/content/spinner.cpp
        src/chatlog/content/spinner.h
        src/chatlog/content/text.cpp
        src/chatlog/content/text.h
        src/chatlog/content/timestamp.cpp
        src/chatlog/content/timestamp.h
        src/chatlog/content/broken.cpp
        src/chatlog/content/broken.h
        src/chatlog/content/contactavatar.cpp
        src/chatlog/content/contactavatar.h
        src/chatlog/content/simpletext.cpp
        src/chatlog/content/simpletext.h
        src/chatlog/customtextdocument.cpp
        src/chatlog/customtextdocument.h
        src/chatlog/documentcache.cpp
        src/chatlog/documentcache.h
        src/chatlog/pixmapcache.cpp
        src/chatlog/pixmapcache.h
        src/chatlog/toxfileprogress.cpp
        src/chatlog/toxfileprogress.h
        src/chatlog/textformatter.cpp
        src/chatlog/textformatter.h
        src/core/coreav.cpp
        src/core/coreav.h
        src/core/core.h
        src/core/core.cpp
        src/core/corefile.cpp
        src/core/corefile.h
        src/core/dhtserver.cpp
        src/core/dhtserver.h
        src/core/icoresettings.h
        src/core/toxcall.cpp
        src/core/toxcall.h
        src/core/toxencrypt.cpp
        src/core/toxencrypt.h
        src/core/toxfile.cpp
        src/core/toxfile.h
        src/core/toxfilepause.h
        src/core/toxid.cpp
        src/core/toxid.h
        src/core/groupid.cpp
        src/core/groupid.h
        src/core/toxoptions.cpp
        src/core/toxoptions.h
        src/core/FriendId.cpp
	src/core/FriendId.h
        src/core/contactid.cpp
        src/core/contactid.h
        src/friendlist.cpp
        src/friendlist.h
        src/grouplist.cpp
        src/grouplist.h
        src/ipc.cpp
        src/ipc.h
        src/model/aboutfriend.cpp
        src/model/aboutfriend.h
        src/model/iaboutfriend.h
        src/model/chatroom/chatroom.h
        src/model/chatroom/chatroom.cpp
        src/model/chatroom/friendchatroom.cpp
        src/model/chatroom/friendchatroom.h
        src/model/chatroom/groupchatroom.cpp
        src/model/chatroom/groupchatroom.h
        src/model/contact.cpp
        src/model/contact.h
        src/model/chatlogitem.cpp
        src/model/chatlogitem.h
        src/model/friend.cpp
        src/model/friend.h
        src/model/message.h
        src/model/message.cpp
        src/model/imessagedispatcher.h
        src/model/friendmessagedispatcher.h
        src/model/friendmessagedispatcher.cpp
        src/model/groupmessagedispatcher.h
        src/model/groupmessagedispatcher.cpp
        src/model/message.h
        src/model/message.cpp
        src/model/groupinvite.cpp
        src/model/groupinvite.h
        src/model/group.cpp
        src/model/group.h
        src/model/status.cpp
        src/model/status.h
        src/model/interface.h
        src/model/profile/iprofileinfo.h
        src/model/profile/profileinfo.cpp
        src/model/profile/profileinfo.h
        src/model/dialogs/idialogs.h
        src/model/ichatlog.h
        src/model/sessionchatlog.h
        src/model/sessionchatlog.cpp
        src/model/chathistory.h
        src/model/chathistory.cpp
        src/model/toxclientstandards.h
        src/nexus.cpp
        src/nexus.h
        src/persistence/db/rawdatabase.cpp
        src/persistence/db/rawdatabase.h
        src/persistence/history.cpp
        src/persistence/history.h
        src/persistence/ifriendsettings.h
        src/persistence/offlinemsgengine.cpp
        src/persistence/offlinemsgengine.h
        src/persistence/paths.cpp
        src/persistence/paths.h
        src/persistence/profile.cpp
        src/persistence/profile.h
        src/persistence/profilelocker.cpp
        src/persistence/profilelocker.h
        src/persistence/serialize.cpp
        src/persistence/serialize.h
        src/persistence/settings.cpp
        src/persistence/settings.h
        src/persistence/settingsserializer.cpp
        src/persistence/settingsserializer.h
        src/persistence/smileypack.cpp
        src/persistence/smileypack.h
        src/persistence/toxsave.cpp
        src/persistence/toxsave.h
        src/video/cameradevice.cpp
        src/video/cameradevice.h
        src/video/camerasource.cpp
        src/video/camerasource.h
        src/video/corevideosource.cpp
        src/video/corevideosource.h
        src/video/genericnetcamview.cpp
        src/video/genericnetcamview.h
        src/video/groupnetcamview.cpp
        src/video/groupnetcamview.h
        src/video/ivideosettings.h
        src/video/netcamview.cpp
        src/video/netcamview.h
        src/video/videoframe.cpp
        src/video/videoframe.h
        src/video/videomode.cpp
        src/video/videomode.h
        src/video/videosource.cpp
        src/video/videosource.h
        src/video/videosurface.cpp
        src/video/videosurface.h
        src/widget/form/aboutfriendform.cpp
        src/widget/form/aboutfriendform.h
#        src/widget/categorywidget.cpp
#        src/widget/categorywidget.h
        src/widget/chatformheader.cpp
        src/widget/chatformheader.h
#        src/widget/circlewidget.cpp
#        src/widget/circlewidget.h
        src/widget/contentdialog.cpp
        src/widget/contentdialog.h
        src/widget/contentdialogmanager.cpp
        src/widget/contentdialogmanager.h
        src/widget/contentlayout.cpp
        src/widget/contentlayout.h
        src/widget/emoticonswidget.cpp
        src/widget/emoticonswidget.h
        src/widget/flowlayout.cpp
        src/widget/flowlayout.h
        src/widget/searchform.cpp
        src/widget/searchform.h
        src/widget/searchtypes.h
        src/widget/ContactWidget.h
        src/widget/ContactWidget.cpp
        src/widget/MainLayout.h
        src/widget/MainLayout.cpp
        src/widget/MessageSessionListWidget.h
        src/widget/MessageSessionListWidget.cpp
        src/widget/MessageSessionWidget.h
        src/widget/MessageSessionWidget.cpp
        src/widget/form/aboutgroupform.h
        src/widget/form/aboutgroupform.cpp
        src/widget/form/addfriendform.cpp
        src/widget/form/addfriendform.h
        src/widget/form/friendform.h
        src/widget/form/friendform.cpp
        src/widget/form/friendform.ui
        src/widget/form/chatform.cpp
        src/widget/form/chatform.h
        src/widget/form/filesform.cpp
        src/widget/form/filesform.h
        src/widget/form/genericchatform.cpp
        src/widget/form/genericchatform.h
        src/widget/form/groupchatform.cpp
        src/widget/form/groupchatform.h
        src/widget/form/groupinviteform.cpp
        src/widget/form/groupinviteform.h
        src/widget/form/groupinvitewidget.cpp
        src/widget/form/groupinvitewidget.h
				src/widget/form/GroupCreateForm.h
				src/widget/form/GroupCreateForm.cpp
				src/widget/form/GroupCreateForm.ui
        src/widget/form/loadhistorydialog.cpp
        src/widget/form/loadhistorydialog.h
        src/widget/form/profileform.cpp
        src/widget/form/profileform.h
        src/widget/form/searchsettingsform.cpp
        src/widget/form/searchsettingsform.h
        src/widget/form/setpassworddialog.cpp
        src/widget/form/setpassworddialog.h
        src/widget/form/settings/aboutform.cpp
        src/widget/form/settings/aboutform.h
        src/widget/form/settings/advancedform.cpp
        src/widget/form/settings/advancedform.h
        src/widget/form/settings/avform.cpp
        src/widget/form/settings/avform.h
        src/widget/form/settings/generalform.cpp
        src/widget/form/settings/generalform.h
        src/widget/form/settings/genericsettings.cpp
        src/widget/form/settings/genericsettings.h
        src/widget/form/settings/privacyform.cpp
        src/widget/form/settings/privacyform.h
        src/widget/form/settings/userinterfaceform.h
        src/widget/form/settings/userinterfaceform.cpp
        src/widget/form/settings/verticalonlyscroller.cpp
        src/widget/form/settings/verticalonlyscroller.h
        src/widget/form/settingswidget.cpp
        src/widget/form/settingswidget.h
        src/widget/form/tabcompleter.cpp
        src/widget/form/tabcompleter.h
    src/widget/form/CallDurationForm.h
    src/widget/form/CallDurationForm.cpp
        src/widget/friendlistlayout.cpp
        src/widget/friendlistlayout.h
        src/widget/friendlistwidget.cpp
        src/widget/friendlistwidget.h
        src/widget/friendwidget.cpp
        src/widget/friendwidget.h
        src/widget/genericchatitemlayout.cpp
        src/widget/genericchatitemlayout.h
        src/widget/genericchatitemwidget.cpp
        src/widget/genericchatitemwidget.h
        src/widget/genericchatroomwidget.cpp
        src/widget/genericchatroomwidget.h
        src/widget/groupwidget.cpp
        src/widget/groupwidget.h
        src/widget/gui.cpp
        src/widget/gui.h
        # src/widget/loginscreen.cpp src/widget/loginscreen.h
        src/widget/maskablepixmapwidget.cpp
        src/widget/maskablepixmapwidget.h
        src/widget/notificationedgewidget.cpp
        src/widget/notificationedgewidget.h
        src/widget/notificationscrollarea.cpp
        src/widget/notificationscrollarea.h
        src/widget/passwordedit.cpp
        src/widget/passwordedit.h
        src/widget/qrwidget.cpp
        src/widget/qrwidget.h
        src/widget/splitterrestorer.cpp
        src/widget/splitterrestorer.h
        src/widget/style.cpp
        src/widget/style.h
        src/widget/tool/activatedialog.cpp
        src/widget/tool/activatedialog.h
        src/widget/tool/adjustingscrollarea.cpp
        src/widget/tool/adjustingscrollarea.h
        src/widget/tool/callconfirmwidget.cpp
        src/widget/tool/callconfirmwidget.h
        src/widget/tool/chattextedit.cpp
        src/widget/tool/chattextedit.h
        src/widget/tool/croppinglabel.cpp
        src/widget/tool/croppinglabel.h
        src/widget/tool/flyoutoverlaywidget.cpp
        src/widget/tool/flyoutoverlaywidget.h
        src/widget/tool/friendrequestdialog.cpp
        src/widget/tool/friendrequestdialog.h
        src/widget/tool/identicon.cpp
        src/widget/tool/identicon.h
        src/widget/tool/movablewidget.cpp
        src/widget/tool/movablewidget.h
        src/widget/tool/profileimporter.cpp
        src/widget/tool/profileimporter.h
        src/widget/tool/recursivesignalblocker.cpp
        src/widget/tool/recursivesignalblocker.h
        src/widget/tool/removefrienddialog.cpp
        src/widget/tool/removefrienddialog.h
        src/widget/tool/screengrabberchooserrectitem.cpp
        src/widget/tool/screengrabberchooserrectitem.h
        src/widget/tool/screengrabberoverlayitem.cpp
        src/widget/tool/screengrabberoverlayitem.h
        src/widget/tool/screenshotgrabber.cpp
        src/widget/tool/screenshotgrabber.h
        src/widget/tool/toolboxgraphicsitem.cpp
        src/widget/tool/toolboxgraphicsitem.h
        src/widget/tool/roundedmaskcontainer.cpp
        src/widget/tool/roundedmaskcontainer.h

        src/widget/ChatWidget.h
        src/widget/ChatWidget.cpp
        src/widget/ContentWidget.cpp
        src/widget/ContentWidget.h
        src/widget/FriendAddWidget.cpp
        src/widget/FriendAddWidget.h
        # src/widget/translator.cpp src/widget/translator.h
        src/widget/widget.cpp
        src/widget/widget.h
        src/worker/SendWorker.h
        src/worker/SendWorker.cpp
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(${PROJECT_NAME}_SOURCES
            ${${PROJECT_NAME}_SOURCES} src/platform/camera/directshow.cpp
            src/platform/camera/directshow.h)
    set(${PROJECT_NAME}_RESOURCES ${${PROJECT_NAME}_RESOURCES})
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux" OR ${CMAKE_SYSTEM_NAME} MATCHES
        "FreeBSD")
    set(${PROJECT_NAME}_SOURCES
            ${${PROJECT_NAME}_SOURCES} src/platform/camera/v4l2.cpp
            src/platform/camera/v4l2.h)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(${PROJECT_NAME}_SOURCES
            ${${PROJECT_NAME}_SOURCES} src/platform/install_osx.cpp
            src/platform/install_osx.h)
endif ()

if (UNIX)
    set(${PROJECT_NAME}_SOURCES
            ${${PROJECT_NAME}_SOURCES} src/platform/posixsignalnotifier.cpp
            src/platform/posixsignalnotifier.h)
endif ()

if (PLATFORM_EXTENSIONS)
    set(${PROJECT_NAME}_SOURCES ${${PROJECT_NAME}_SOURCES}
            src/platform/capslock.h src/platform/timer.h)

    if (WIN32)
        set(${PROJECT_NAME}_SOURCES
                ${${PROJECT_NAME}_SOURCES} src/platform/capslock_win.cpp
                src/platform/timer_win.cpp)
    elseif (${X11_EXT})
        set(${PROJECT_NAME}_SOURCES
                ${${PROJECT_NAME}_SOURCES} src/platform/capslock_x11.cpp
                src/platform/timer_x11.cpp src/platform/x11_display.cpp)
    elseif (${APPLE_EXT})
        set(${PROJECT_NAME}_SOURCES
                ${${PROJECT_NAME}_SOURCES} src/platform/capslock_osx.cpp
                src/platform/timer_osx.cpp
 )
    endif ()
endif ()

qt5_add_translation(
        ${PROJECT_NAME}_QM_FILES
        translations/de.ts
        translations/es.ts
        translations/fr.ts
        translations/it.ts
        translations/ja.ts
        translations/ko.ts
        translations/pt.ts
        translations/ru.ts
        translations/ar.ts
        translations/zh_CN.ts
        translations/zh_TW.ts)

set(RESOURCE_NAME "${PROJECT_BINARY_DIR}/${PROJECT_NAME}.qrc.in")

file(WRITE "${RESOURCE_NAME}"
        "<!DOCTYPE RCC>
<RCC version=\"1.0\">
  <qresource prefix=\"/translations/${PROJECT_NAME}\">
")

foreach (qm ${${PROJECT_NAME}_QM_FILES})
    get_filename_component(qm_name ${qm} NAME)
    file(APPEND "${RESOURCE_NAME}"
            "    <file alias=\"${qm_name}\">${qm}</file>\n")
endforeach (qm)

file(APPEND "${RESOURCE_NAME}"
        "</qresource>
</RCC>
")

execute_process(
        COMMAND
        ${CMAKE_COMMAND} -E copy_if_different
        ${RESOURCE_NAME}
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.qrc)

if (NOT SMILEYS)
    set(SMILEYS "")
endif ()

set(SMILEY_RESOURCES "")
if (NOT "${SMILEYS}" STREQUAL "DISABLED")
    set(SMILEY_RESOURCES ${PROJECT_SOURCE_DIR}/smileys/emojione.qrc)

    if (NOT "${SMILEYS}" STREQUAL "MIN")
        set(SMILEY_RESOURCES ${SMILEY_RESOURCES}
                ${PROJECT_SOURCE_DIR}/smileys/smileys.qrc)
    endif ()
endif ()

qt5_add_resources(
        ${PROJECT_NAME}_RESOURCES
        res.qrc
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.qrc
        ${SMILEY_RESOURCES}
        # DEPENDS ${${PROJECT_NAME}_QM_FILES} # OPTIONS ${RCC_OPTIONS}
)


add_library(${PROJECT_NAME} STATIC
        ${${PROJECT_NAME}_FORMS}
        ${${PROJECT_NAME}_SOURCES}
        ${${PROJECT_NAME}_QM_FILES}
        ${${PROJECT_NAME}_RESOURCES})

if (WIN32)
  target_include_directories(${PROJECT_NAME}
        PRIVATE
            ${AVCODEC_INCLUDE_DIRS}
            ${AVFORMAT_INCLUDE_DIRS}
            ${AVUTIL_INCLUDE_DIRS}
            ${SWSCALE_INCLUDE_DIRS}
        )
        

    set(PLATFORM_LIBS d3d11 dxgi dxva2 gdi32
                        wmcodecdspuuid Winmm Mf Mfplat Mfreadwrite mfuuid evr 
                        amstrmid msdmo strmiids strmbase
                        winspool comdlg32 uuid shlwapi iphlpapi 
                        kernel32 user32 Vfw32 shell32 oleaut32 ole32 advapi32
    )

    if(CMAKE_BUILD_TYPE MATCHES "Debug")
        set(PLATFORM_LIBS ${PLATFORM_LIBS} MSVCRTD)
    else()
        set(PLATFORM_LIBS ${PLATFORM_LIBS} MSVCRT)
    endif()
elseif (UNIX)
    set(PLATFORM_LIBS z OpenSSL::SSL OpenSSL::Crypto atomic X11)
endif ()

target_link_libraries(${PROJECT_NAME}
    PUBLIC ${ok-gloox_LIBRARIES}
    PUBLIC ${Qt5Core_LIBRARIES}
    PUBLIC ${Qt5Gui_LIBRARIES}
    PUBLIC ${Qt5Widgets_LIBRARIES}
    PUBLIC ${Qt5Network_LIBRARIES}
    PUBLIC ${Qt5Xml_LIBRARIES}
    PUBLIC ${Qt5Svg_LIBRARIES}
    PUBLIC ${CMAKE_REQUIRED_LIBRARIES}
    PUBLIC ${ALL_LIBRARIES}
    PUBLIC ${PLATFORM_LIBS}
)

if (MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif (MSVC)
