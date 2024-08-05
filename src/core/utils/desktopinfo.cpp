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

#include "desktopinfo.h"
#include <QProcessEnvironment>

DesktopInfo::DesktopInfo() {
    auto e = QProcessEnvironment::systemEnvironment();
    XDG_CURRENT_DESKTOP = e.value(QStringLiteral("XDG_CURRENT_DESKTOP"));
    XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));
    KDE_FULL_SESSION = e.value(QStringLiteral("KDE_FULL_SESSION"));
    GNOME_DESKTOP_SESSION_ID = e.value(QStringLiteral("GNOME_DESKTOP_SESSION_ID"));
    DESKTOP_SESSION = e.value(QStringLiteral("DESKTOP_SESSION"));
}

bool DesktopInfo::waylandDetected() {
    return XDG_SESSION_TYPE == QLatin1String("wayland") ||
           WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive);
}

DesktopInfo::WM DesktopInfo::windowManager() {
    DesktopInfo::WM res = DesktopInfo::OTHER;
    QStringList desktops = XDG_CURRENT_DESKTOP.split(QChar(':'));
    for (auto& desktop : desktops) {
        if (desktop.contains(QLatin1String("GNOME"), Qt::CaseInsensitive)) {
            return DesktopInfo::GNOME;
        }
        if (desktop.contains(QLatin1String("sway"), Qt::CaseInsensitive)) {
            return DesktopInfo::SWAY;
        }
        if (desktop.contains(QLatin1String("kde-plasma"))) {
            return DesktopInfo::KDE;
        }
    }

    if (!GNOME_DESKTOP_SESSION_ID.isEmpty()) {
        return DesktopInfo::GNOME;
    }

    if (!KDE_FULL_SESSION.isEmpty()) {
        return DesktopInfo::KDE;
    }

    return res;
}
