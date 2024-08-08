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

using HttpErrorFn = ok::base::Fn<void(int statusCode, const QByteArray body)>;
using HttpDownloadProgressFn = ok::base::Fn<void(qint64 bytesReceived, qint64 bytesTotal)>;
using HttpUploadProgressFn = ok::base::Fn<void(qint64 bytesSent, qint64 bytesTotal)>;
using HttpBodyFn = ok::base::Fn<void(QByteArray body, QString filename)>;
using HttpJsonBodyFn = ok::base::Fn<void(QJsonDocument json, QString filename)>;

class NetworkHttp : public QObject {
    Q_OBJECT

public:
    explicit NetworkHttp(QObject* parent = nullptr);
    ~NetworkHttp() override;

    void setHeader(QString k, QString v){
        headers.insert(k, v);
    };

    void setHeaders(const QMap<QString, QString> map){
        headers.insert(map);
    }

    void httpFinished();

    bool get(const QUrl& url,
             const HttpBodyFn& fn,
             const HttpDownloadProgressFn& progress = nullptr,
             const HttpErrorFn& failed = nullptr);

    QByteArray get(const QUrl& url, const HttpDownloadProgressFn& downloadProgress = nullptr);

    bool getJson(const QUrl& url,
                 ok::base::Fn<void(QJsonDocument)> fn = nullptr,
                 const HttpErrorFn& err = nullptr);

    bool post(const QUrl& url,
              const QByteArray& data,
              const QString& contentType,
              const HttpBodyFn& fn = nullptr,
              const HttpDownloadProgressFn& progress = nullptr,
              const HttpUploadProgressFn& upload = nullptr,
              const HttpErrorFn& failed = nullptr);

    bool postJson(const QUrl& url,
                  const QJsonDocument& data,
                  const HttpBodyFn& fn = nullptr,
                  const HttpDownloadProgressFn& progress = nullptr,
                  const HttpUploadProgressFn& upload = nullptr,
                  const HttpErrorFn& failed = nullptr);

    bool postJson1(const QUrl& url,
                  const QJsonDocument& data,
                  const HttpJsonBodyFn& fn = nullptr,
                  const HttpDownloadProgressFn& progress = nullptr,
                  const HttpUploadProgressFn& upload = nullptr,
                  const HttpErrorFn& failed = nullptr);

    void PostFormData(const QUrl& url, QFile* file, const HttpUploadProgressFn& uploadProgress,
                      ok::base::Fn<void(const QJsonObject& json)> readyRead);

    virtual void PostFormData(const QUrl& url,
                              const QByteArray& byteArray,
                              const QString& contentType,
                              const QString& filename,
                              const HttpUploadProgressFn& uploadProgress,
                              ok::base::Fn<void(const QJsonObject&)>
                                      readyRead);

    void doRequest(QNetworkRequest& req, QNetworkReply* reply, const HttpBodyFn& fn = nullptr,
                   const HttpDownloadProgressFn& = nullptr, const HttpUploadProgressFn& = nullptr,
                   const HttpErrorFn& = nullptr);

    void forRequest(QNetworkRequest &req);

protected:
    QNetworkAccessManager* _manager;
    QMap<QString, QString> headers;
};
}  // namespace network
