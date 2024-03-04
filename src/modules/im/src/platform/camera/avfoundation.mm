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

#include "avfoundation.h"
#include <QObject>

#import <AVFoundation/AVFoundation.h>

QVector<QPair<QString, QString> > avfoundation::getDeviceList()
{
    QVector<QPair<QString, QString> > result;

#if MACOS_VERSION_MAJOR > 10 || (MACOS_VERSION_MAJOR == 10 && MACOS_VERSION_MINOR > 14)
    AVCaptureDevice* device = [AVCaptureDevice defaultDeviceWithMediaType:AVMediaTypeVideo];
    id objects[] = {device};
    NSUInteger count = sizeof(objects) / sizeof(id);
    NSArray* devices = [NSArray arrayWithObjects:objects count:count];
#else
    NSArray* devices = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo];
#endif

    for (AVCaptureDevice* device in devices) {
        result.append({ QString::fromNSString([device uniqueID]), QString::fromNSString([device localizedName]) });
    }

    uint32_t numScreens = 0;
    CGGetActiveDisplayList(0, NULL, &numScreens);
    if (numScreens > 0) {
        CGDirectDisplayID screens[numScreens];
        CGGetActiveDisplayList(numScreens, screens, &numScreens);
        for (uint32_t i = 0; i < numScreens; i++) {
            result.append({ QString("%1 %2").arg(CAPTURE_SCREEN).arg(i), QObject::tr("Capture screen %1").arg(i) });
        }
    }

    return result;
}

QVector<VideoMode> avfoundation::getDeviceModes(QString devName)
{
    QVector<VideoMode> result;

    if (devName.startsWith(CAPTURE_SCREEN)) {
        return result;
    }
    else {
        NSString* deviceName = [NSString stringWithCString:devName.toUtf8() encoding:NSUTF8StringEncoding];
        AVCaptureDevice* device = [AVCaptureDevice deviceWithUniqueID:deviceName];

        if (device == nil) {
            return result;
        }

        for (AVCaptureDeviceFormat* format in [device formats]) {
            CMFormatDescriptionRef formatDescription;
            CMVideoDimensions dimensions;
            formatDescription = (CMFormatDescriptionRef)[format performSelector:@selector(formatDescription)];
            dimensions = CMVideoFormatDescriptionGetDimensions(formatDescription);

            for (AVFrameRateRange* range in format.videoSupportedFrameRateRanges) {
                VideoMode mode;
                mode.width = dimensions.width;
                mode.height = dimensions.height;
                mode.FPS = range.maxFrameRate;
                result.append(mode);
            }
        }
    }

    return result;
}
