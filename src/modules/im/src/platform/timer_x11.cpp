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
// clang-format off
#include <QDebug>
#include <X11/extensions/scrnsaver.h>
#include "src/platform/timer.h"
#include "src/platform/x11_display.h"
// clang-format on

uint32_t Platform::getIdleTime() {
    uint32_t idleTime = 0;

    Display* display = X11Display::lock();
    if (!display) {
        qWarning() << "XOpenDisplay failed";
        X11Display::unlock();
        return 0;
    }

    int32_t x11event = 0, x11error = 0;
    static int32_t hasExtension = XScreenSaverQueryExtension(display, &x11event, &x11error);
    if (hasExtension) {
        XScreenSaverInfo* info = XScreenSaverAllocInfo();
        if (info) {
            XScreenSaverQueryInfo(display, DefaultRootWindow(display), info);
            idleTime = info->idle;
            XFree(info);
        } else
            qWarning() << "XScreenSaverAllocInfo() failed";
    }
    X11Display::unlock();
    return idleTime;
}
