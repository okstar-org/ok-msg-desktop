
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

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QMutex>
#include <QMutexLocker>
#include <QString>
#include <QThread>
#include <QtGlobal>
#include <memory>
#include "basic_types.h"

#ifndef __FILE_NAME__
#define __FILE_NAME__ (strrchr("/" __FILE__, '/') + 1)
#endif

namespace Logs {

#define L_DEBUG "D"
#define L_INFO "I"
#define L_WARN "W"
#define L_ERROR "E"

void writeDebug(const char* file, int line, const char* func, const QString& str);

void writeDebug(const char* file, int line, const char* func, const std::string& str);

}  // namespace Logs
