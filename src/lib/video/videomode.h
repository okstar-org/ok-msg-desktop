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

#pragma once

#include <cstdint>
#include <QString>
#include <QRect>
#include <QObject>


namespace lib::video {

//Video source type
enum class VideoType {
    //摄像头
    Camera,
    //桌面
    Desktop,
    //文件
    File,
    //流
    Stream
};


struct VideoMode {
    int width = 0;
    int height = 0;
    int x = 0;
    int y = 0;
    float FPS = -1.0f;
    uint32_t pixel_format = 0;

    VideoMode(int width = 0, int height = 0, int x = 0, int y = 0, float FPS = -1.0f);

    explicit VideoMode(QRect rect);

    inline bool isValid() const {
        return width > 0 && height > 0;
    }

    QRect toRect() const;

    operator bool() const;
    bool operator==(const VideoMode& other) const;
    uint32_t norm(const VideoMode& other) const;
    uint32_t tolerance() const;

    QString toString() const;
};


struct VideoDevice{
    VideoType type;
    QString name;
    QString url;
};

}  // namespace lib::video

Q_DECLARE_METATYPE(lib::video::VideoType);


