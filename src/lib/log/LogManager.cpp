/*
 * Copyright (c) 2022 船山信息 chuanshaninfo.com
 * The project is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PubL v2 for more details.
 */

//
// Created by gaojie on 24-4-19.
//

#include "LogManager.h"
#include "base/OkProcess.h"
#include "base/OkSettings.h"
#include "base/times.h"

#include <QAtomicPointer>
#include <QDebug>
#include <QDir>
#include <QMutexLocker>
#include <QThread>

namespace ok {
namespace lib {

static QAtomicPointer<QFile> logFilePointer;
static QMutex logMutex;

void logMessageHandler(QtMsgType type, const QMessageLogContext& ctxt, const QString& msg) {
    QMutexLocker locker(&logMutex);

    QString file = ctxt.file;
    if (file.lastIndexOf("/") > 0) {
        file = file.mid(file.lastIndexOf('/') + 1);
    }

    // Time should be in UTC to save user privacy on log sharing
    QString time = ok::base::Times::formatTime(ok::base::Times::now(), "yy-MM-dd HH:mm:ss.zzz");
    QString line = QString("[%1] [%2] [%3:%4] - ")
                           .arg(time)
                           .arg(QThread::currentThread()->objectName())
                           .arg(file)
                           .arg(ctxt.line);
    switch (type) {
        case QtDebugMsg:
            line += "Debug";
            break;
        case QtInfoMsg:
            line += "Info";
            break;
        case QtWarningMsg:
            line += "Warning";
            break;
        case QtCriticalMsg:
            line += "Critical";
            break;
        case QtFatalMsg:
            line += "Fatal";
            break;
        default:
            break;
    }

    line += ": " + msg + "\n";
    QByteArray lineBytes = line.toUtf8();
    fwrite(lineBytes.constData(), 1, lineBytes.size(), stdout);
    fflush(stdout);
    QFile* logFilePtr = logFilePointer.loadRelaxed();
    if (logFilePtr) {
        logFilePtr->write(lineBytes);
        logFilePtr->flush();
    }
}

static LogManager* log = nullptr;

LogManager::LogManager() {
    qDebug() << "Initialize LogManager";
    logFileDir = base::OkSettings::getAppLogPath();
    qDebug() << "Log file dir is:" << logFileDir;

    logName = APPLICATION_NAME "-" +
              ::ok::base::Times::formatTime(::ok::base::Times::now(), "yyyyMMddHHmmss") + "-" +
              QString::number(base::OkProcess::selfPid()) + ".log";

    QString logFilePath = logFileDir.path() + QDir::separator() + logName;
    qDebug() << "Log file is:" << logFilePath;
    file = std::make_unique<QFile>(logFilePath);
    file->open(QIODevice::ReadWrite);
    logFilePointer.storeRelaxed(file.get());
    qInstallMessageHandler(logMessageHandler);
}

LogManager::~LogManager() { Destroy(); }

const LogManager& LogManager::Instance() {
    if (!log) {
        log = new LogManager();
    }
    return *log;
}

void LogManager::Destroy() {
    delete log;
    log = nullptr;
}

}  // namespace lib
}  // namespace ok
