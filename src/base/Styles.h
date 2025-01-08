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


#ifndef STYLES_H
#define STYLES_H

#include <QWidget>

namespace base {


class Styles{

public:
    static constexpr const QSize AVATAR_SIZE{40, 40};

    static void repolish(QWidget* w) {
    if(!w) return;

    w->style()->unpolish(w);
    w->style()->polish(w);

    for (QObject* o : w->children()) {
        QWidget* c = qobject_cast<QWidget*>(o);
        if (c) {
            c->style()->unpolish(c);
            c->style()->polish(c);
        }
    }
}

};

}
#endif  // STYLES_H
