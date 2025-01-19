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

#ifndef CONTACTAVATAR_H
#define CONTACTAVATAR_H

#include <QPixmap>
#include "../chatlinecontent.h"
#include "base/compatiblerecursivemutex.h"
namespace module::im {
class ContactAvatar : public ChatLineContent {
public:
    explicit ContactAvatar(const QPixmap& avatar);

    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    void setWidth(qreal width) override;
    [[nodiscard]] QRectF boundingRect() const override;
    [[nodiscard]] qreal getAscent() const override;
    const void* getContent() override;
    void setPixmap(const QPixmap& avatar);

protected:
    void onCopyEvent() override;

private:
    static QIcon invalidAvatar();
    QPixmap avatar;
    CompatibleRecursiveMutex mutex;
};
}  // namespace module::im
#endif  // CONTACTAVATAR_H
