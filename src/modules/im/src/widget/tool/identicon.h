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

#ifndef IDENTICON_H
#define IDENTICON_H

#include <QColor>
#include <QImage>

class Identicon {
public:
    Identicon(const QByteArray& data);
    QImage toImage(int scaleFactor = 1);

public:
    static constexpr int IDENTICON_ROWS = 5;

private:
    float bytesToColor(QByteArray bytes);

private:
    static constexpr int COLORS = 2;
    static constexpr int ACTIVE_COLS = (IDENTICON_ROWS + 1) / 2;
    static constexpr int IDENTICON_COLOR_BYTES = 6;
    static constexpr int HASH_MIN_LEN =
            ACTIVE_COLS * IDENTICON_ROWS + COLORS * IDENTICON_COLOR_BYTES;

    uint8_t identiconColors[IDENTICON_ROWS][ACTIVE_COLS];
    QColor colors[COLORS];
};

#endif  // IDENTICON_H
