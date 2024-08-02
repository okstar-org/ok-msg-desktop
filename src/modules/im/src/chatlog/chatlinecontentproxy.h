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

#ifndef CHATLINECONTENTPROXY_H
#define CHATLINECONTENTPROXY_H

#include <QGraphicsProxyWidget>
#include "chatlinecontent.h"

class FileTransferWidget;

class ChatLineContentProxy : public ChatLineContent {
    Q_OBJECT

public:
    enum ChatLineContentProxyType {
        GenericType,
        FileTransferWidgetType = 0,
    };

public:
    ChatLineContentProxy(QWidget* widget, int minWidth, float widthInPercent = 1.0f);
    ChatLineContentProxy(FileTransferWidget* widget, int minWidth, float widthInPercent = 1.0f);

    QRectF boundingRect() const override;
    void setWidth(qreal width) override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
    qreal getAscent() const override;

    QWidget* getWidget() const;
    ChatLineContentProxyType getWidgetType() const;

protected:
    ChatLineContentProxy(QWidget* widget, ChatLineContentProxyType type, int minWidth,
                         float widthInPercent);

private:
    QGraphicsProxyWidget* proxy;
    float widthPercent;
    int widthMin;
    const ChatLineContentProxyType widgetType;
};

#endif  // CHATLINECONTENTPROXY_H
