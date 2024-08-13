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

#ifndef I_VIDEO_SETTINGS_H
#define I_VIDEO_SETTINGS_H

#include "src/base/interface.h"

#include <QRect>
#include <QString>

class IVideoSettings {
public:
    virtual ~IVideoSettings() = default;

    virtual QString getVideoDev() const = 0;
    virtual void setVideoDev(const QString& deviceSpecifier) = 0;

    virtual QRect getScreenRegion() const = 0;
    virtual void setScreenRegion(const QRect& value) = 0;

    virtual bool getScreenGrabbed() const = 0;
    virtual void setScreenGrabbed(bool value) = 0;

    virtual QRect getCamVideoRes() const = 0;
    virtual void setCamVideoRes(QRect newValue) = 0;

    virtual float getCamVideoFPS() const = 0;
    virtual void setCamVideoFPS(float newValue) = 0;

    DECLARE_SIGNAL(videoDevChanged, const QString& device);
    DECLARE_SIGNAL(screenRegionChanged, const QRect& region);
    DECLARE_SIGNAL(screenGrabbedChanged, bool enabled);
    DECLARE_SIGNAL(camVideoResChanged, const QRect& region);
    DECLARE_SIGNAL(camVideoFPSChanged, unsigned short fps);
};

#endif  // I_VIDEO_SETTINGS_H
