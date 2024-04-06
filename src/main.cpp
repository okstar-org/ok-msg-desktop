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

#include "base/OkProcess.h"
#include "base/OkSettings.h"
#include "base/r.h"
#include "base/times.h"
#include "launcher.h"
#include <QDebug>
#include <QDir>
#include <QMutex>
#include <QThread>
#include <QTime>

#ifdef LOG_TO_FILE
static QAtomicPointer<QFile> logFileFile;
void logMessageHandler(QtMsgType type, const QMessageLogContext& ctxt, const QString& msg)
{
  // Silence qWarning spam due to bug in QTextBrowser (trying to open a file for base64 images)
  if (ctxt.function == QString("virtual bool QFSFileEngine::open(QIODevice::OpenMode)")
      && msg == QString("QFSFileEngine::open: No file name specified"))
    return;

  QString file = ctxt.file;
  if (file.lastIndexOf("/")>0) {
    file = file.mid(file.lastIndexOf('/') + 1);
  }

  // Time should be in UTC to save user privacy on log sharing
  QDateTime time = QDateTime::currentDateTime();
  QString line = QString("[%1] [%2] [%3:%4] - ")
                       .arg(time.toString("yy-MM-dd HH:mm:ss.zzz"))
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
  QByteArray LogMsgBytes = line.toUtf8();
  fwrite(LogMsgBytes.constData(), 1, LogMsgBytes.size(), stdout);

  QFile* logFilePtr = logFileFile.loadRelaxed();
  logFilePtr->write(msg.toUtf8());
}
#endif

int main(int argc, char *argv[]) {

  Q_INIT_RESOURCE(resources);

#ifdef LOG_TO_FILE

  auto& settings = ok::base::OkSettings::getInstance();
  QString logFileDir = settings.getAppCacheDirPath();
  qDebug()<<"Log file dir is:"<<logFileDir;

  QDir logDir(logFileDir);
  if(!logDir.exists()){
    logDir.mkpath(".");
  }

  QString log = logFileDir
                + APPLICATION_NAME + "-"
                +base::Times::now().toString("yyyyMMddHHmmss") + "-"
                +QString::number(ok::base::OkProcess::selfPid()) + ".log";
  qDebug()<<"Log file is:"<< log;
  auto file=new QFile(log);
  file->open(QIODevice::ReadWrite);
  logFileFile.storeRelaxed(file);

  qInstallMessageHandler(logMessageHandler);

#endif

  const auto launcher = core::Launcher::Create(argc, argv);
  return launcher ? launcher->startup() : 1;

}
