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

#include "videomode.h"

/**
 * @struct VideoMode
 * @brief Describes a video mode supported by a device.
 *
 * @var unsigned short VideoMode::width, VideoMode::height
 * @brief Displayed video resolution (NOT frame resolution).
 *
 * @var unsigned short VideoMode::x, VideoMode::y
 * @brief Coordinates of upper-left corner.
 *
 * @var float VideoMode::FPS
 * @brief Frames per second supported by the device at this resolution
 * @note a value < 0 indicates an invalid value
 */

VideoMode::VideoMode(int width, int height, int x, int y, float FPS)
        : width(width), height(height), x(x), y(y), FPS(FPS) {}

VideoMode::VideoMode(QRect rect)
        : width(rect.width()), height(rect.height()), x(rect.x()), y(rect.y()) {}

QRect VideoMode::toRect() const { return QRect(x, y, width, height); }

bool VideoMode::operator==(const VideoMode& other) const {
    return width == other.width && height == other.height && x == other.x && y == other.y &&
           qFuzzyCompare(FPS, other.FPS) && pixel_format == other.pixel_format;
}
// 标准
uint32_t VideoMode::norm(const VideoMode& other) const {
    return qAbs(this->width - other.width) + qAbs(this->height - other.height);
}
// 容忍
uint32_t VideoMode::tolerance() const {
    constexpr uint32_t minTolerance = 300;  // keep wider tolerance for low res cameras
    constexpr uint32_t toleranceFactor =
            10;  // video mode must be within 10% to be "close enough" to ideal
    return std::max((width + height) / toleranceFactor, minTolerance);
}

/**
 * @brief All zeros means a default/unspecified mode
 */
VideoMode::operator bool() const { return width || height || static_cast<int>(FPS); }
