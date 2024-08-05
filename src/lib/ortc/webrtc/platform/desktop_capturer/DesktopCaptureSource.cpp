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

#include "DesktopCaptureSource.h"
#include <string>

namespace lib::ortc {

std::string DesktopCaptureSourceData::cachedKey() const {
    return std::to_string(aspectSize.width) + 'x' + std::to_string(aspectSize.height) + ':' +
           std::to_string(fps) + ':' + (captureMouse ? '1' : '0');
}

DesktopCaptureSource::DesktopCaptureSource(long long uniqueId, std::string title, bool isWindow)
        : _uniqueId(uniqueId), _title(std::move(title)), _isWindow(isWindow) {}

long long DesktopCaptureSource::uniqueId() const { return _uniqueId; }

bool DesktopCaptureSource::isWindow() const { return _isWindow; }

std::string DesktopCaptureSource::deviceIdKey() const {
    return std::string("desktop_capturer_") + (_isWindow ? "window_" : "screen_") +
           std::to_string(uniqueId());
}

std::string DesktopCaptureSource::title() const { return _isWindow ? _title : "Screen"; }

std::string DesktopCaptureSource::uniqueKey() const {
    return std::to_string(_uniqueId) + ':' + (_isWindow ? "Window" : "Screen");
}

std::string DesktopCaptureSource::deviceIdKey() {
    return static_cast<const DesktopCaptureSource*>(this)->deviceIdKey();
}

std::string DesktopCaptureSource::title() {
    return static_cast<const DesktopCaptureSource*>(this)->title();
}

std::string DesktopCaptureSource::uniqueKey() {
    return static_cast<const DesktopCaptureSource*>(this)->uniqueKey();
}

}  // namespace lib::ortc
