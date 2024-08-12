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
#ifndef COLORHELPER_H
#define COLORHELPER_H

#include <QColor>
#include <QString>

namespace base {

class ColorHelper {
public:
    enum FORMAT { RGBA, RGB, HEX };

    static QString makeColorString(const QColor& color, const FORMAT type = HEX) {
        switch (type) {
            case HEX:
                return color.name().toUpper();
                //            return QString().sprintf("#%02X%02X%02X",
                //                                     color.red(),
                //                                     color.green(),
                //                                     color.blue());
                //                    .arg(color.alpha() != 255 ?
                //                    QString().sprintf("%02X", color.alpha()) :
                //                    QString());
                //            break;
            case RGB:
                return QString("rgb(%1, %2, %3)")
                        .arg(color.red())
                        .arg(color.green())
                        .arg(color.blue());
                //            break;
            case RGBA:

                return QString("rgba(%1, %2, %3, %4)")
                        .arg(color.red())
                        .arg(color.green())
                        .arg(color.blue())
                        .arg(color.alpha());
                //            break;
        }

        return color.name();
    }
};

}  // namespace base

#endif  // COLORHELPER_H
