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

#include <memory>

#include <QByteArray>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>

#include <QHttpMultiPart>
#include <QNetworkAccessManager>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSslSocket>
#include <QTimer>
#include <QUrlQuery>

#include <base/files.h>
#include "NetworkHttp.h"
#include "base/jsons.h"

namespace network {

NetworkHttp::NetworkHttp(QObject* parent) : QObject(parent), _manager{nullptr} {
    qDebug() << __func__;
#ifndef QT_NO_SSL
    QString buildVersion = QSslSocket::sslLibraryBuildVersionString();
    qDebug() << "The build-in ssl library version is:" << buildVersion;

    bool supportsSsl = QSslSocket::supportsSsl();
    qDebug() << "Detected ssl:" << supportsSsl;
    if (supportsSsl) {
        QString libraryVersion = QSslSocket::sslLibraryVersionString();
        qDebug() << "libraryVersion:" << libraryVersion;
    }
#endif
    //  QNetworkAccessManager
    _manager = new QNetworkAccessManager(this);
    auto schemes = _manager->supportedSchemes();
    qDebug() << "supportedSchemes:" << schemes.join(" ");
}

NetworkHttp::~NetworkHttp() { qDebug() << __func__; }

inline void forRequest(QNetworkRequest& req) {
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, true);
}

bool NetworkHttp::get(const QUrl& url,
                      const HttpBodyFn& fn,
                      const HttpDownloadProgressFn& progress,
                      const HttpErrorFn& failed) {
    qDebug() << "Url:" << url.toString();
    if (url.isEmpty()) {
        qWarning() << "url is empty";
        return false;
    }

    QNetworkRequest req(url);
    forRequest(req);

    auto* reply = _manager->get(req);
    doRequest(req, reply, fn, progress, nullptr, failed);
    return reply;
}

QByteArray NetworkHttp::get(const QUrl& url, const HttpDownloadProgressFn& downloadProgress) {
    qDebug() << "Url:" << url.toString();
    auto _reply = _manager->get(QNetworkRequest(url));
    _reply->ignoreSslErrors();
    if (!_reply->errorString().isEmpty()) {
        return {};
    }

    if (downloadProgress) {
        connect(_reply, &QNetworkReply::downloadProgress, downloadProgress);
    }

    QEventLoop loop;
    connect(_reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QByteArray byteArr = _reply->readAll();
    int size = byteArr.size();
    qDebug() << ("Received bytes:") << (size);
    return byteArr;
}

bool NetworkHttp::getJSON(const QUrl& url, Fn<void(QJsonDocument)> fn, const HttpErrorFn& errFn) {
    return get(
            url,
            [=](QByteArray buf, QString fileName) {
                Q_UNUSED(fileName)
                fn(Jsons::toJSON(buf));
            },
            nullptr, errFn);
}

void NetworkHttp::postJSON(const QUrl& url,
                           const QJsonObject& data,
                           Fn<void(const QJsonDocument&)>
                                   fn,
                           const HttpDownloadProgressFn& progress,
                           const HttpUploadProgressFn& upload,
                           const HttpErrorFn& failed) {
    post(
            url, QJsonDocument(data).toJson(), "application/json",
            [&](QByteArray buf, QString name) { fn(Jsons::toJSON(buf)); }, progress, upload,
            failed);
}

void NetworkHttp::post(const QUrl& url,
                       const QByteArray& data,
                       const QString& contentType,
                       const HttpBodyFn& fn,
                       const HttpDownloadProgressFn& progress,
                       const HttpUploadProgressFn& upload,
                       const HttpErrorFn& failed) {
    qDebug() << __func__ << url.toString();

    QNetworkRequest req(url);
    forRequest(req);
    req.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(contentType));

    auto _reply = _manager->post(req, data);
    doRequest(req, _reply, fn, progress, upload, failed);
}

void NetworkHttp::PostFormData(const QUrl& url,
                               const QByteArray& byteArray,
                               const QString& contentType,
                               const QString& filename,
                               const HttpUploadProgressFn& uploadProgress,
                               Fn<void(const QJsonObject&)>
                                       readyRead) {
    if (url.isEmpty()) {
        qWarning() << "url is empty!";
        return;
    }

    if (byteArray.size() <= 0) {
        qWarning() << "byteArray is empty!";
        return;
    }
    // 添加认证信息
    auto* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    /* type */
    QUrlQuery uQuery(url.query());
    // uQuery.addQueryItem("type", (UploadFileFolders[(int)folder]));
    const_cast<QUrl&>(url).setQuery(uQuery);

    // file
    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(contentType));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QVariant("form-data; name=\"file\"; filename=\"" + filename + "\""));

    imagePart.setBody(byteArray);
    multiPart->append(imagePart);

    QNetworkRequest req(url);
    forRequest(req);

    QList<QNetworkCookie> cookies = _manager->cookieJar()->cookiesForUrl(url);
    // cookies.append(QNetworkCookie(QString("ticket").toUtf8(),
    // client->token().toUtf8()));
    QVariant var;
    var.setValue(cookies);

    req.setHeader(QNetworkRequest::CookieHeader, var);
    QNetworkReply* reply = _manager->post(req, multiPart);
    multiPart->setParent(reply);

    if (uploadProgress) {
        connect(reply, &QNetworkReply::uploadProgress, uploadProgress);
    }
}

void NetworkHttp::doRequest(QNetworkRequest& req,
                            QNetworkReply* reply,
                            const HttpBodyFn& fn,
                            const HttpDownloadProgressFn& progress,
                            const HttpUploadProgressFn& upload,
                            const HttpErrorFn& failed) {
    qDebug() << __func__;

    if (!reply) {
        return;
    }
    reply->ignoreSslErrors();

    connect(reply, &QNetworkReply::finished, [=]() {
        // 获取HTTP状态码
        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        if (statusCode.isValid()) {
            qDebug() << "statusCode:" << statusCode.toInt();
        }

        if (statusCode.toInt() / 100 != 2) {
            qWarning() << "Error:" << reply->errorString();
            if (failed) {
                failed(statusCode.toInt(), reply->errorString());
            }
            return;
        }

        auto bytes = reply->readAll();
        auto size = bytes.size();
        qDebug() << "Received bytes:" << size;

        if (size <= 0) {
            qWarning() << "No content!";
            return;
        }

        if (!fn) {
            qWarning() << "Not put callback for content!";
            return;
        }

        auto cth = reply->header(QNetworkRequest::KnownHeaders::ContentTypeHeader);
        if (cth.isValid()) {
            auto type = cth.toString();
            qDebug() << "content-type:" << type;
            if (type.startsWith("text/", Qt::CaseInsensitive) ||
                type.startsWith("application/json", Qt::CaseInsensitive)) {
                qDebug() << qstring("body:%1").arg(QString::fromUtf8(bytes));
            }
        }

        auto cdh = reply->header(QNetworkRequest::KnownHeaders::ContentDispositionHeader);
        if (!cdh.isNull()) {
            QString filename = cdh.toString().split("=").last().trimmed();
            fn(bytes, filename);
        } else {
            fn(bytes, req.url().fileName());
        }
        // delete reply
        reply->deleteLater();
    });

    if (progress) {
        connect(reply, &QNetworkReply::downloadProgress, progress);
    }

    if (upload) {
        connect(reply, &QNetworkReply::uploadProgress, upload);
    }
}

/**上传表单数据
 * @brief NetworkHttp::PostFile
 * @param url
 * @param file
 * @param fn
 */
void NetworkHttp::PostFormData(const QUrl& url, QFile* file,
                               const HttpUploadProgressFn& uploadProgress,
                               Fn<void(const QJsonObject&)> readyRead) {
    if (url.isEmpty()) {
        qWarning() << "url is empty!";
        return;
    }

    if (!file) {
        qWarning() << "file is nullptr!";
        return;
    }

    QString contentType = ok::base::Files::GetContentTypeStr(file->fileName());
    file->open(QIODevice::ReadOnly);

    QByteArray byteArray = file->readAll();
    PostFormData(url, byteArray, ok::base::Files::GetContentTypeStr(file->fileName()),
                 file->fileName(), uploadProgress, readyRead);
}

void NetworkHttp::httpFinished() { qDebug() << "finished."; }

}  // namespace network
