
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
#include "logs.h"
#include "qthread.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QIODevice>
#include <QMutex>
#include <QString>

namespace Logs {

void _logsWrite(const QString& msg) { qDebug().noquote() << msg; }

void writeDebug(const char* file, int line, const char* func, const std::string& str) {
    writeDebug(file, line, func, qstring(str));
}

void writeDebug(const char* file, int line, const char* func, const QString& v) {
    QString msg(QString("LOG:%1 [%2] (%3:%4) %5 %6")
                        .arg(QDateTime::currentDateTime().toString(Qt::DateFormat::ISODate))
                        .arg(QThread::currentThread()->objectName())
                        .arg(file)
                        .arg(line)
                        .arg(func)
                        .arg(v));

    _logsWrite(msg);
}

}  // namespace Logs
