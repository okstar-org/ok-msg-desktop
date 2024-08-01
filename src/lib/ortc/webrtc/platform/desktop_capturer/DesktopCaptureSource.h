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

#ifndef OkMSG_DESKTOP_CAPTURE_SOURCE_H__
#define OkMSG_DESKTOP_CAPTURE_SOURCE_H__

#include <string>

#ifdef WEBRTC_WIN
// Compiler errors in conflicting Windows headers if not included here.
#include <winsock2.h>
#endif  // WEBRTC_WIN

namespace lib::ortc {

class VideoSource {
public:
    virtual ~VideoSource() = default;

    virtual std::string deviceIdKey() = 0;
    virtual std::string title() = 0;
    virtual std::string uniqueKey() = 0;
};

struct DesktopSize {
    int width = 0;
    int height = 0;
};

struct DesktopCaptureSourceData {
    DesktopSize aspectSize;
    double fps = 24.;
    bool captureMouse = true;

    std::string cachedKey() const;
};

class DesktopCaptureSource : public VideoSource {
public:
    DesktopCaptureSource(long long uniqueId, std::string title, bool isWindow);

    static DesktopCaptureSource Invalid() { return InvalidTag{}; }

    long long uniqueId() const;
    bool isWindow() const;

    std::string deviceIdKey() const;
    std::string title() const;
    std::string uniqueKey() const;

    bool valid() const { return _valid; }
    explicit operator bool() const { return _valid; }

private:
    struct InvalidTag {};
    DesktopCaptureSource(InvalidTag) : _valid(false) {}

    std::string deviceIdKey() override;
    std::string title() override;
    std::string uniqueKey() override;

    long long _uniqueId = 0;
    std::string _title;
    bool _isWindow = false;
    bool _valid = true;
};

}  // namespace lib::ortc

#endif  // TGCALLS_DESKTOP_CAPTURE_SOURCE_H__
