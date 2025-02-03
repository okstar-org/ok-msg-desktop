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

#include "NetworkHttp.h"

#include <memory>
#include <QObject>
#include <QString>
#include <QUrl>

namespace lib::network {

ImageLoader::ImageLoader(QObject* parent) : QObject(parent) {
    http = new NetworkHttp(this);
}

ImageLoader::~ImageLoader() = default;

bool ImageLoader::load(const QString& url, const ok::base::Fn<void(QByteArray body, QString name)>& fn) {
   return http->get(QUrl(url), fn);
}

}  // namespace lib::network
