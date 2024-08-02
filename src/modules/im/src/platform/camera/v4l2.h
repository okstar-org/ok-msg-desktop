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
#ifndef V4L2_H
#define V4L2_H

#include <QPair>
#include <QString>
#include <QVector>
#include "src/video/videomode.h"

#if !(defined(Q_OS_LINUX) || defined(Q_OS_FREEBSD))
#error "This file is only meant to be compiled for Linux or FreeBSD targets"
#endif

namespace v4l2 {
QVector<VideoMode> getDeviceModes(QString devName);
QVector<QPair<QString, QString>> getDeviceList();
QString getPixelFormatString(uint32_t pixel_format);
bool betterPixelFormat(uint32_t a, uint32_t b);
}  // namespace v4l2

#endif  // V4L2_H
