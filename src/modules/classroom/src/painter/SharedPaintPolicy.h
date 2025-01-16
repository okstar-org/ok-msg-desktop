/*
 * Copyright (c) 2022 船山信息 chuanshaninfo.com
 * The project is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PubL v2 for more details.
 */

#pragma once

#define VERSION_TEXT "0.12.0"
#define PROTOCOL_VERSION_TEXT "0.0.1"

#define AUTHOR_TEXT "gunoodaddy"
#define PROGRAME_TEXT "Shared Painter"

#ifdef Q_WS_WIN
#define PROGRAM_FILE_NAME "SharedPainter.exe"
#define PROGRAM_UPGRADER_FILE_NAME "Upgrader.exe"
#else
#define PROGRAM_FILE_NAME "SharedPainter"
#define PROGRAM_UPGRADER_FILE_NAME "Upgrader"
#endif

#define DEFAULT_RECORD_FILE_PATH "record"
#define DEFAULT_AUTO_SAVE_FILE_PATH "autosave"
#define DEFAULT_AUTO_SAVE_FILE_NAME_PREFIX "auto_saved_"

#define NET_MAGIC_CODE 0xBEBE

#define MAX_PACKET_BODY_SIZE 200000000  // 2OOMB

#define DEFAULT_RECONNECT_TRY_COUNT 3

#define DEFAULT_UPGRADE_CHECK_SECOND 20
#define DEFAULT_UPGRADE_FILE_NAME "patch.zip"

#define DEFAULT_TEXT_ITEM_POS_REGION_W 9999
#define DEFAULT_TEXT_ITEM_POS_REGION_H 300

#define DEFAULT_PIXMAP_ITEM_SIZE_W 250

#define DEFAULT_TRAY_MESSAGE_DURATION_MSEC 5000
#define DEFAULT_GRID_LINE_SIZE_W 32
#define FINDING_SERVER_TRY_COUNT 20

#define DEFAULT_INITIAL_CHATWINDOW_SIZE 240

#ifdef Q_WS_WIN
#define NATIVE_NEWLINE_STR "\r\n"
#else
#define NATIVE_NEWLINE_STR "\n"
#endif
