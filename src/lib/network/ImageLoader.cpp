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

#include "ImageLoader.h"

#include <memory>

#include <QObject>
#include <QString>
#include <QUrl>

#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace utils {

ImageLoader::ImageLoader(QObject* parent) : QObject(parent) {}

ImageLoader::~ImageLoader() {}

void ImageLoader::load(const QString& url, ok::base::Fn<void(const QByteArray&)> fn) {
    QNetworkAccessManager manager;
    QEventLoop loop;

    QNetworkReply* reply = manager.get(QNetworkRequest(url));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    fn(reply->readAll());
}

}  // namespace utils
