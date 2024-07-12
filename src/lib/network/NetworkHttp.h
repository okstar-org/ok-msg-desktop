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

#include <QByteArray>
#include <QFile>
#include <QJsonObject>

#include <QObject>
#include <QString>
#include <QUrl>
#include <memory>

#include <base/basic_types.h>
#include <base/timer.h>

class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;

namespace network {

using HttpErrorFn = Fn<void(int statusCode, const QString &errStr)>;
using HttpDownloadProgressFn = Fn<void(qint64 bytesReceived, qint64 bytesTotal)>;
using HttpUploadProgressFn = Fn<void(qint64 bytesSent, qint64 bytesTotal)>;
using HttpBodyFn = Fn<void(QByteArray body, QString filename)>;

class NetworkHttp : public QObject {
  Q_OBJECT

public:
  explicit NetworkHttp(QObject *parent = nullptr);
  ~NetworkHttp() override;

  void httpFinished();

  bool get(const QUrl &url,
           const HttpBodyFn &fn,
           const HttpDownloadProgressFn &progress = nullptr,
           const HttpErrorFn &failed = nullptr);

  QByteArray get(const QUrl &url,
                 const Fn<void(qint64 bytesReceived, qint64 bytesTotal)> &downloadProgress = nullptr);

  bool getJSON(const QUrl &url, Fn<void(QJsonDocument)> fn = nullptr, const HttpErrorFn& err = nullptr);

  void post(const QUrl &url,
            const QByteArray &data,
            const QString& contentType,
            const HttpBodyFn &fn = nullptr,
            const HttpDownloadProgressFn &progress= nullptr,
            const HttpUploadProgressFn& upload= nullptr,
            const HttpErrorFn &failed= nullptr);

  QByteArray post(const QUrl &url, const QString &str);

  void postJSON(const QUrl &url,
                const QJsonObject &data,
                Fn<void(const QJsonDocument &)> fn,
          const HttpDownloadProgressFn &progress= nullptr,
          const HttpUploadProgressFn& upload= nullptr,
          const HttpErrorFn &failed= nullptr);

  void PostFormData(const QUrl &url, QFile *file, Fn<void(int bytesSent, int bytesTotal)> uploadProgress, Fn<void(const QJsonObject &json)> readyRead);

  virtual void PostFormData(const QUrl &url, const QByteArray &byteArray, const QString &contentType, const QString &filename, Fn<void(int, int)> uploadProgress,
                            Fn<void(const QJsonObject &)> readyRead);

  void doRequest(QNetworkRequest &req, QNetworkReply *reply,
                 const HttpBodyFn &fn = nullptr,
                 const HttpDownloadProgressFn & = nullptr,
                 const HttpUploadProgressFn& = nullptr,
                 const HttpErrorFn & = nullptr);

protected:
  QNetworkAccessManager *_manager;
};
} // namespace network
