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

#ifndef DESKTOPNOTIFY_H
#define DESKTOPNOTIFY_H

#include <libsnore/snore.h>

#include <QObject>
#include <memory>

class DesktopNotify : public QObject {
    Q_OBJECT
public:
    DesktopNotify();

    enum class MessageType { FRIEND, FRIEND_FILE, FRIEND_REQUEST, GROUP, GROUP_INVITE };

public slots:
    void notifyMessage(const QString& title, const QString& message);
    void notifyMessagePixmap(const QString& title, const QString& message, QPixmap avatar);
    void notifyMessageSimple(const MessageType type);

private:
    void createNotification(const QString& title, const QString& text, Snore::Icon& icon);

private:
    Snore::SnoreCore& notifyCore;
    Snore::Application snoreApp;
    Snore::Icon snoreIcon;
};

#endif  // DESKTOPNOTIFY_H
