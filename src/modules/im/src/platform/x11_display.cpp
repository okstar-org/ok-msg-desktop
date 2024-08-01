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

#include "src/platform/x11_display.h"
#include <QtCore/qsystemdetection.h>
#include <X11/Xlib.h>
#include <QMutex>

namespace Platform {

struct X11DisplayPrivate {
    Display* display;
    QMutex mutex;

    X11DisplayPrivate() : display(XOpenDisplay(nullptr)) {}
    ~X11DisplayPrivate() {
        if (display) {
            XCloseDisplay(display);
        }
    }
    static X11DisplayPrivate& getSingleInstance() {
        // object created on-demand
        static X11DisplayPrivate singleInstance;
        return singleInstance;
    }
};

Display* X11Display::lock() {
    X11DisplayPrivate& singleInstance = X11DisplayPrivate::getSingleInstance();
    singleInstance.mutex.lock();
    return singleInstance.display;
}

void X11Display::unlock() { X11DisplayPrivate::getSingleInstance().mutex.unlock(); }
}  // namespace Platform
