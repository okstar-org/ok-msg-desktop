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

#include "screengrabber.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QGuiApplication>
#include <QPixmap>
#include <QScreen>
#include "filenamehandler.h"

#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
#include <QDBusInterface>
#include <QDBusReply>
#endif

ScreenGrabber::ScreenGrabber(QObject* parent) : QObject(parent) {}

QPixmap ScreenGrabber::grabEntireDesktop(bool& ok) {
    ok = true;
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    if (m_info.waylandDetected()) {
        QPixmap res;
        // handle screenshot based on DE
        switch (m_info.windowManager()) {
            case DesktopInfo::GNOME: {
                // https://github.com/GNOME/gnome-shell/blob/695bfb96160033be55cfb5ac41c121998f98c328/data/org.gnome.Shell.Screenshot.xml
                QString path = FileNameHandler().properScreenshotPath(QDir::tempPath()) + ".png";
                QDBusInterface gnomeInterface(QStringLiteral("org.gnome.Shell"),
                                              QStringLiteral("/org/gnome/Shell/Screenshot"),
                                              QStringLiteral("org.gnome.Shell.Screenshot"));
                QDBusReply<bool> reply = gnomeInterface.call("Screenshot", false, false, path);
                if (reply.value()) {
                    res = QPixmap(path);
                } else {
                    ok = false;
                }
                break;
            }
            case DesktopInfo::KDE: {
                // https://github.com/KDE/spectacle/blob/517a7baf46a4ca0a45f32fd3f2b1b7210b180134/src/PlatformBackends/KWinWaylandImageGrabber.cpp#L145
                QDBusInterface kwinInterface(QStringLiteral("org.kde.KWin"),
                                             QStringLiteral("/Screenshot"),
                                             QStringLiteral("org.kde.kwin.Screenshot"));
                QDBusReply<QString> reply = kwinInterface.call("screenshotFullscreen");
                res = QPixmap(reply.value());
                break;
            }
            default:
                ok = false;
                break;
        }
        if (!ok) {
            // SystemNotification().sendMessage(tr("Unable to capture screen"));
        }
        return res;
    }
#endif

    QRect geometry;
    for (QScreen* const screen : QGuiApplication::screens()) {
        geometry = geometry.united(screen->geometry());
    }

    QPixmap p(QApplication::primaryScreen()->grabWindow(QApplication::desktop()->winId(),
                                                        geometry.x(), geometry.y(),
                                                        geometry.width(), geometry.height()));
    p.setDevicePixelRatio(QApplication::desktop()->devicePixelRatio());
    return p;
}

QPixmap ScreenGrabber::grabScreen(int screenNumber, bool& ok) {
    QPixmap p;
    bool isVirtual = QApplication::desktop()->isVirtualDesktop();
    if (isVirtual) {
        p = grabEntireDesktop(ok);
        if (ok) {
            QPoint topLeft(0, 0);
#ifdef Q_OS_WIN
            for (QScreen* const screen : QGuiApplication::screens()) {
                QPoint topLeftScreen = screen->geometry().topLeft();
                if (topLeft.x() > topLeftScreen.x() || topLeft.y() > topLeftScreen.y()) {
                    topLeft = topLeftScreen;
                }
            }
#endif
            QRect geometry = QApplication::desktop()->screenGeometry(screenNumber);
            geometry.moveTo(geometry.topLeft() - topLeft);
            p = p.copy(geometry);
        }
    } else {
        p = QApplication::desktop()->screen(screenNumber)->grab();
        ok = true;
    }
    return p;
}
