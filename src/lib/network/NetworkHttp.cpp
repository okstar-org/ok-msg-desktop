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

#include "NetworkHttp.h"

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

#include "base/jsons.h"
#include <base/files.h>
#include <base/logs.h>

namespace network {

NetworkHttp::NetworkHttp(QObject *parent) : QObject(parent) {
   bool supportsSsl = QSslSocket::supportsSsl();
   DEBUG_LOG(("supportsSsl:%1").arg(supportsSsl));
   QString buildVersion = QSslSocket::sslLibraryBuildVersionString();
   DEBUG_LOG(("buildVersion:%1").arg(buildVersion));
   QString libraryVersion = QSslSocket::sslLibraryVersionString();
   DEBUG_LOG(("libraryVersion:%1").arg(libraryVersion));

  //  QNetworkAccessManager
  _manager = std::make_unique<QNetworkAccessManager>(this);
  auto schemes = _manager->supportedSchemes();
  DEBUG_LOG(("supportedSchemes:%1").arg(schemes.join(" ")));
}

NetworkHttp::~NetworkHttp() {}

bool NetworkHttp::get(
    const QUrl &url,                                            //
    Fn<void(QByteArray body, const QString filename)> fn,       //
    Fn<void(qint64 bytesReceived, qint64 bytesTotal)> progress, //
    Fn<void(const QString &errStr)> failed) {

  DEBUG_LOG(("Url:%1").arg(url.toString()));
  if(url.isEmpty())
  {
    qWarning()<<"url is empty";
    return false;
  }

  auto *reply = _manager->get(QNetworkRequest(url));
  reply->ignoreSslErrors();

  connect(reply, &QNetworkReply::finished, [=]() {
    if (reply->error()) {
      qWarning() << "Error:" << reply->errorString();
      if (failed)
        failed("网络连接错误");
      return;
    }
    auto bytes = reply->readAll();
    auto size = bytes.size();
    qDebug() << "Received bytes:" << size << QString(bytes);
    if (size <= 0)
      return;

    if (!fn)
      return;

    auto header = reply->header(QNetworkRequest::KnownHeaders::ContentDispositionHeader);
    if (!header.isNull()) {
      QString filename = header.toString().split("=").last().trimmed();
      fn(bytes, filename);
    } else {
      fn(bytes, url.fileName());
    }
    //delete reply
//    reply->deleteLater();
  });

  if (progress) {
    connect(reply, &QNetworkReply::downloadProgress, progress);
  }

  return true;
}

QByteArray NetworkHttp::get(
    const QUrl &url,
    const Fn<void(qint64 bytesReceived, qint64 bytesTotal)> &downloadProgress) {
  DEBUG_LOG(("Url:%1").arg(url.toString()));

  QNetworkRequest request(url);
  wrapRequest(request, url.host());

  auto _reply = _manager->get(request);
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
  DEBUG_LOG(("Received bytes:%1").arg(size));
  return byteArr;
}

bool NetworkHttp::getJSON(const QUrl &url,
                          Fn<void(QJsonDocument)> fn,
                          Fn<void(const QString &)> errFn) {
  return get(
      url,
      [=](const QByteArray &buf, const QString &fileName) {
        Q_UNUSED(fileName)
        fn(Jsons::toJSON(buf));
      },
      nullptr,
      [=](const QString& err) {
        Q_UNUSED(err)
        if (errFn)
          errFn(err);
      });

}

void NetworkHttp::postJSON(const QUrl &url, const QJsonObject &data,
                           Fn<void(const QJsonDocument &)> fn) {
  post(url, QString(QJsonDocument(data).toJson()),
       [&](const QByteArray buf) { fn(Jsons::toJSON(buf)); });
}

void NetworkHttp::postJSON(const QUrl &url, const QString &data,
                           Fn<void(const QJsonDocument &)> fn) {
  post(url, data, [&](QByteArray buf) { fn(Jsons::toJSON(buf)); });
}

void NetworkHttp::post(const QUrl &url, const QString &data,
                       Fn<void(QByteArray)> fn) {
  DEBUG_LOG(("Url:%1").arg(url.toString()));

  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    QVariant("application/json"));
  wrapRequest(request, url.host());

  auto postData = QByteArray::fromStdString(data.toStdString());
  auto _reply = _manager->post(request, postData);
  _reply->ignoreSslErrors();

  QEventLoop loop;
  connect(_reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  QByteArray byteArr = _reply->readAll();
  DEBUG_LOG(("Received bytes:%1").arg(byteArr.size()));
  fn(byteArr);
}

QByteArray NetworkHttp::post(const QUrl &url, const QString &data) {
  DEBUG_LOG(("Url:%1").arg(url.toString()));

  if (data.isEmpty()) {
    DEBUG_LOG_S(L_WARN) << ("data isEmpty.");
    return QByteArray::fromStdString("");
  }

  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    QVariant("application/json"));
  request.setRawHeader("Accept", "application/json");
  wrapRequest(request, url.host());

  auto postData = QByteArray::fromStdString(data.toStdString());
  QNetworkReply *_reply = (_manager->post(request, postData));
  _reply->ignoreSslErrors();

  connect(_reply, &QNetworkReply::finished, this, &NetworkHttp::httpFinished);

  QEventLoop loop;
  connect(_reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  QByteArray byteArr = _reply->readAll();
  return (byteArr);
}

void NetworkHttp::PostFormData(
    const QUrl &url, const QByteArray &byteArray, const QString &contentType,
    const QString &filename,
    Fn<void(int bytesSent, int bytesTotal)> uploadProgress,
    Fn<void(const QJsonObject &)> readyRead) {

  if (url.isEmpty()) {
    DEBUG_LOG_S(L_ERROR) << "url is empty!";
    return;
  }

  if (byteArray.size() <= 0) {
    DEBUG_LOG_S(L_ERROR) << "byteArray is empty!";
    return;
  }

  DEBUG_LOG_S(L_INFO) << "url:" << url << "byteArray:" << byteArray.size();

  // 添加认证信息

  QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

  /* type */
  QUrlQuery uQuery(url.query());
  // uQuery.addQueryItem("type", (UploadFileFolders[(int)folder]));
  const_cast<QUrl &>(url).setQuery(uQuery);

  // file
  QHttpPart imagePart;
  imagePart.setHeader(QNetworkRequest::ContentTypeHeader,
                      QVariant(contentType));
  imagePart.setHeader(
      QNetworkRequest::ContentDispositionHeader,
      QVariant("form-data; name=\"file\"; filename=\"" + filename + "\""));

  imagePart.setBody(byteArray);
  multiPart->append(imagePart);

  QNetworkRequest request(url);

  QList<QNetworkCookie> cookies = _manager->cookieJar()->cookiesForUrl(url);
  // cookies.append(QNetworkCookie(QString("ticket").toUtf8(),
  // client->token().toUtf8()));
  QVariant var;
  var.setValue(cookies);

  for (auto &c : cookies) {
    DEBUG_LOG_S(L_INFO) << "cookie name: " << c.name()
                        << " value: " << c.value();
  }

  request.setHeader(QNetworkRequest::CookieHeader, var);
  QNetworkReply *reply = _manager->post(request, multiPart);
  multiPart->setParent(reply);

  if(uploadProgress){
    connect(reply, &QNetworkReply::uploadProgress, uploadProgress);
  }
}

/**上传表单数据
 * @brief NetworkHttp::PostFile
 * @param url
 * @param file
 * @param fn
 */
void NetworkHttp::PostFormData(
    const QUrl &url, QFile *file,
    Fn<void(int bytesSent, int bytesTotal)> uploadProgress,
    Fn<void(const QJsonObject &)> readyRead) {
  if (url.isEmpty()) {
    DEBUG_LOG_S(L_ERROR) << "url is empty!";
    return;
  }

  if (!file) {
    DEBUG_LOG_S(L_ERROR) << "file is nullptr!";
    return;
  }

  DEBUG_LOG_S(L_INFO) << "url:" << url << "file:" << file->fileName();

  QString contentType = base::Files::GetContentTypeStr(file->fileName());

  file->open(QIODevice::ReadOnly);

  QByteArray byteArray = file->readAll();

  PostFormData(url, byteArray, base::Files::GetContentTypeStr(file->fileName()),
               file->fileName(), uploadProgress, readyRead);
}

void NetworkHttp::wrapRequest(const QNetworkRequest &request, const QUrl &url) {

  auto cs = _manager->cookieJar()->cookiesForUrl(url);
  if (!cs.empty()) {
    for (auto &c : cs) {
      DEBUG_LOG_S(L_INFO) << "cookie: " << c.domain() << "/" << c.path() << "|"
                          << c.name() << "=" << c.value();
    }
  }
}

void NetworkHttp::httpFinished() { DEBUG_LOG_S(L_INFO) << "..."; }
} // namespace network
