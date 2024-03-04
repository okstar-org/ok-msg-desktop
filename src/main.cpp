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


#include <QDir>
#include <QMutex>
#include <QTime>
#include <QThread>
#include <QDebug>
#include "base/r.h"
#include "launcher.h"

#ifdef LOG_TO_FILE
static QAtomicPointer<FILE> logFileFile = nullptr;
// Store log messages until log file opened
QMutex* logBufferMutex = new QMutex();
static QList<QByteArray>* logBuffer = new QList<QByteArray>();

void logMessageHandler(QtMsgType type, const QMessageLogContext& ctxt, const QString& msg)
{
  // Silence qWarning spam due to bug in QTextBrowser (trying to open a file for base64 images)
  if (ctxt.function == QString("virtual bool QFSFileEngine::open(QIODevice::OpenMode)")
      && msg == QString("QFSFileEngine::open: No file name specified"))
    return;

  QString file = ctxt.file;
  // We're not using QT_MESSAGELOG_FILE here, because that can be 0, NULL, or
  // nullptr in release builds.
//  QString path = QString(__FILE__);
//  path = path.left(path.lastIndexOf('/') + 1);
  if (file.lastIndexOf("/")>0) {
    file = file.mid(file.lastIndexOf('/') + 1);
  }

  // Time should be in UTC to save user privacy on log sharing
  QDateTime time = QDateTime::currentDateTime();
  QString LogMsg = QString("[%1] [%2] [%3:%4] - ")
                       .arg(time.toString("yy-MM-dd HH:mm:ss.zzz"))
                       .arg(QThread::currentThread()->objectName())
                       .arg(file)
                       .arg(ctxt.line);
  switch (type) {
  case QtDebugMsg:
    LogMsg += "Debug";
    break;
  case QtInfoMsg:
    LogMsg += "Info";
    break;
  case QtWarningMsg:
    LogMsg += "Warning";
    break;
  case QtCriticalMsg:
    LogMsg += "Critical";
    break;
  case QtFatalMsg:
    LogMsg += "Fatal";
    break;
  default:
    break;
  }

  LogMsg += ": " + msg + "\n";
  QByteArray LogMsgBytes = LogMsg.toUtf8();
  fwrite(LogMsgBytes.constData(), 1, LogMsgBytes.size(), stdout);

#ifdef LOG_TO_FILE
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
  FILE* logFilePtr = logFileFile.loadRelaxed(); // atomically load the file pointer
#else
  FILE* logFilePtr = logFileFile.load(); // atomically load the file pointer
#endif
  if (!logFilePtr) {
    logBufferMutex->lock();
    if (logBuffer)
      logBuffer->append(LogMsgBytes);

    logBufferMutex->unlock();
  } else {
    logBufferMutex->lock();
    if (logBuffer) {
      // empty logBuffer to file
      foreach (QByteArray msg, *logBuffer)
        fwrite(msg.constData(), 1, msg.size(), logFilePtr);

      delete logBuffer; // no longer needed
      logBuffer = nullptr;
    }
    logBufferMutex->unlock();

    fwrite(LogMsgBytes.constData(), 1, LogMsgBytes.size(), logFilePtr);
    fflush(logFilePtr);
  }
#endif
}

#endif

int main(int argc, char *argv[]) {

  Q_INIT_RESOURCE(resources);

#ifdef LOG_TO_FILE
  qInstallMessageHandler(logMessageHandler);

//  Settings& settings = Settings::getInstance();
  QString logFileDir = ""; //settings.getAppCacheDirPath();
  QDir(logFileDir).mkpath(".");

  QString logfile = logFileDir + (APPLICATION_NAME ".log");
  FILE* mainLogFilePtr = fopen(logfile.toLocal8Bit().constData(), "a");

  // Trim log file if over 1MB
  if (QFileInfo(logfile).size() > 1000000) {
    qDebug() << "Log file over 1MB, rotating...";

    // close old logfile (need for windows)
    if (mainLogFilePtr)
      fclose(mainLogFilePtr);

    QDir dir(logFileDir);

    // Check if log.1 already exists, and if so, delete it
    if (dir.remove(logFileDir + APPLICATION_NAME ".log.1"))
      qDebug() << "Removed old log successfully";
    else
      qWarning() << "Unable to remove old log file";

    if (!dir.rename(logFileDir + APPLICATION_NAME ".log", logFileDir + APPLICATION_NAME ".log.1"))
      qCritical() << "Unable to move logs";

    // open a new logfile
    mainLogFilePtr = fopen(logfile.toLocal8Bit().constData(), "a");
  }

  if (!mainLogFilePtr)
    qCritical() << "Couldn't open logfile" << logfile;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
  logFileFile.storeRelaxed(mainLogFilePtr); // atomically set the logFile
#else
  logFileFile.store(mainLogFilePtr); // atomically set the logFile
#endif

#endif

  const auto launcher = core::Launcher::Create(argc, argv);
  return launcher ? launcher->startup() : 1;

}
