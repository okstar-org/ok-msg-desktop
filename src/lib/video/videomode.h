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

#ifndef VIDEOMODE_H
#define VIDEOMODE_H

#include <QRect>
#include <cstdint>

struct VideoMode {
    int width, height;
    int x, y;
    float FPS = -1.0f;
    uint32_t pixel_format = 0;

    VideoMode(int width = 0, int height = 0, int x = 0, int y = 0, float FPS = -1.0f);

    explicit VideoMode(QRect rect);

    QRect toRect() const;

    operator bool() const;
    bool operator==(const VideoMode& other) const;
    uint32_t norm(const VideoMode& other) const;
    uint32_t tolerance() const;
};

#endif  // VIDEOMODE_H
