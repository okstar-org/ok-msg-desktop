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

#include "desktopnotify.h"

#include <src/persistence/settings.h>

#include <libsnore/snore.h>

#include <QDebug>

DesktopNotify::DesktopNotify()
        : notifyCore{Snore::SnoreCore::instance()}, snoreIcon{":/img/icons/qtox.svg"} {
    notifyCore.loadPlugins(Snore::SnorePlugin::Backend);
    qDebug() << "primary notification backend:" << notifyCore.primaryNotificationBackend();

    snoreApp = Snore::Application("qTox", snoreIcon);

    notifyCore.registerApplication(snoreApp);
}

void DesktopNotify::createNotification(const QString& title, const QString& text,
                                       Snore::Icon& icon) {
    const Settings& s = Settings::getInstance();
    if (!(s.getNotify() && s.getDesktopNotify())) {
        return;
    }

    Snore::Notification notify{snoreApp, Snore::Alert(), title, text, icon};

    notifyCore.broadcastNotification(notify);
}

void DesktopNotify::notifyMessage(const QString& title, const QString& message) {
    createNotification(title, message, snoreIcon);
}

void DesktopNotify::notifyMessagePixmap(const QString& title, const QString& message,
                                        QPixmap avatar) {
    Snore::Icon new_icon(avatar);
    createNotification(title, message, new_icon);
}

void DesktopNotify::notifyMessageSimple(const MessageType type) {
    QString message;
    switch (type) {
        case MessageType::FRIEND:
            message = tr("New message");
            break;
        case MessageType::FRIEND_FILE:
            message = tr("Incoming file transfer");
            break;
        case MessageType::FRIEND_REQUEST:
            message = tr("Friend request received");
            break;
        case MessageType::GROUP:
            message = tr("New group message");
            break;
        case MessageType::GROUP_INVITE:
            message = tr("Group invite received");
            break;
        default:
            break;
    }

    createNotification(message, {}, snoreIcon);
}
