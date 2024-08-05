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

#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <QtCore/qsystemdetection.h>
#include "src/platform/timer.h"

uint32_t Platform::getIdleTime() {
    // https://hg.pidgin.im/pidgin/main/file/13e4ae613a6a/pidgin/gtkidle.c
    // relevant code introduced to Pidgin in:
    // https://hg.pidgin.im/pidgin/main/diff/8ff1c408ef3e/src/gtkidle.c
    static io_service_t service = 0;
    CFTypeRef property;
    uint64_t idleTime_ns = 0;

    if (!service) {
        mach_port_t master;
        IOMasterPort(MACH_PORT_NULL, &master);
        service = IOServiceGetMatchingService(master, IOServiceMatching("IOHIDSystem"));
    }

    property =
            IORegistryEntryCreateCFProperty(service, CFSTR("HIDIdleTime"), kCFAllocatorDefault, 0);
    CFNumberGetValue((CFNumberRef)property, kCFNumberSInt64Type, &idleTime_ns);
    CFRelease(property);

    return idleTime_ns / 1000000;
}
