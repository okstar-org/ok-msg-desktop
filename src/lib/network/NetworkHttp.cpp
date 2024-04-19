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
#ifndef QT_NO_SSL
   bool supportsSsl = QSslSocket::supportsSsl();
   qDebug()<<("supportsSsl    :")<<(supportsSsl);
   QString buildVersion = QSslSocket::sslLibraryBuildVersionString();
   qDebug()<<("buildVersion   :")<<(buildVersion);
   QString libraryVersion = QSslSocket::sslLibraryVersionString();
   qDebug()<<("libraryVersion :")<<(libraryVersion);
#endif
  //  QNetworkAccessManager
  _manager = std::make_unique<QNetworkAccessManager>(this);
  auto schemes = _manager->supportedSchemes();
  qDebug()<<("supportedSchemes:")<<(schemes.join(" "));
}

NetworkHttp::~NetworkHttp() {}

bool NetworkHttp::get(
    const QUrl &url,                                            //
    Fn<void(QByteArray body, const QString filename)> fn,       //
    Fn<void(qint64 bytesReceived, qint64 bytesTotal)> progress, //
    Fn<void(const QString &errStr)> failed) {

  qDebug() << "Url:" << url.toString();
  if(url.isEmpty())
  {
    qWarning()<<"url is empty";
    return false;
  }
  QNetworkRequest req(url);
  req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, true);
  auto *reply = _manager->get(req);
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
    qDebug() << "Received bytes:" << size;
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
  qDebug() << "Url:" << url.toString();

  QNetworkRequest request(url);

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
  qDebug()<<("Received bytes:")<<(size);
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
  qDebug()<< ("Url:")<<(url.toString());

  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    QVariant("application/json"));


  auto postData = QByteArray::fromStdString(data.toStdString());
  auto _reply = _manager->post(request, postData);
  _reply->ignoreSslErrors();

  QEventLoop loop;
  connect(_reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
  loop.exec();

  QByteArray byteArr = _reply->readAll();
  qDebug() << "Received bytes:%1" << byteArr.size();
  fn(byteArr);
}

QByteArray NetworkHttp::post(const QUrl &url, const QString &data) {
  qDebug() << "Url:" << url.toString();

  if (data.isEmpty()) {
    qWarning() << "data isEmpty!";
    return QByteArray::fromStdString("");
  }

  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader,
                    QVariant("application/json"));
  request.setRawHeader("Accept", "application/json");

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
    qWarning() << "url is empty!";
    return;
  }

  if (byteArray.size() <= 0) {
    qWarning() << "byteArray is empty!";
    return;
  }
  // 添加认证信息
  auto *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

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

} // namespace network
