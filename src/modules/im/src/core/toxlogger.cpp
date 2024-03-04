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

#include "toxlogger.h"

#include <QDebug>
#include <QRegularExpression>
#include <QString>
#include <QStringBuilder>

namespace ToxLogger {
namespace {

QByteArray cleanPath(const char *file)
{
    // for privacy, make the path relative to the c-toxcore source directory
    const QRegularExpression pathCleaner(QLatin1String{"[\\s|\\S]*c-toxcore."});
    QByteArray cleanedPath = QString{file}.remove(pathCleaner).toUtf8();
    cleanedPath.append('\0');
    return cleanedPath;
}

}  // namespace

/**
 * @brief Log message handler for toxcore log messages
 * @note See tox.h for the parameter definitions
 */
void onLogMessage(Tox *tox, Tox_Log_Level level, const char *file, uint32_t line,
                  const char *func, const char *message, void *user_data)
{
    const QByteArray cleanedPath = cleanPath(file);

    switch (level) {
    case TOX_LOG_LEVEL_TRACE:
        return; // trace level generates too much noise to enable by default
    case TOX_LOG_LEVEL_DEBUG:
        QMessageLogger(cleanedPath.data(), line, func).debug() << message;
        break;
    case TOX_LOG_LEVEL_INFO:
        QMessageLogger(cleanedPath.data(), line, func).info() << message;
        break;
    case TOX_LOG_LEVEL_WARNING:
        QMessageLogger(cleanedPath.data(), line, func).warning() << message;
        break;
    case TOX_LOG_LEVEL_ERROR:
        QMessageLogger(cleanedPath.data(), line, func).critical() << message;
        break;
    }
}

}  // namespace ToxLogger
