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
#ifndef AVFOUNDATION_H
#define AVFOUNDATION_H

#include <QPair>
#include <QString>
#include <QVector>
#include "lib/video/videomode.h"

#ifndef Q_OS_MACX
#error "This file is only meant to be compiled for Mac OS X targets"
#endif

namespace avfoundation {
const QString CAPTURE_SCREEN{"Capture screen"};
QVector<lib::video::VideoMode> getDeviceModes(QString devName);
QVector<QPair<QString, QString>> getDeviceList();
}  // namespace avfoundation

#endif  // AVFOUNDATION_H
