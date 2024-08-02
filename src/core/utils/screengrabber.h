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

#ifndef SCREENGRABBER_H
#define SCREENGRABBER_H

#include <QObject>
#include "desktopinfo.h"

#include <QDir>
#include <QScreen>

class ScreenGrabber : public QObject {
    Q_OBJECT
public:
    explicit ScreenGrabber(QObject* parent = nullptr);
    QPixmap grabEntireDesktop(bool& ok);
    QPixmap grabScreen(int screenNumber, bool& ok);

private:
    DesktopInfo m_info;
};
#endif
