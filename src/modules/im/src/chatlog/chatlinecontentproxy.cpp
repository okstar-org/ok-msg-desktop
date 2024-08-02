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

#include "chatlinecontentproxy.h"
#include <QDebug>
#include <QLayout>
#include <QPainter>
#include <QWidget>
#include "src/chatlog/content/filetransferwidget.h"

/**
 * @enum ChatLineContentProxy::ChatLineContentProxyType
 * @brief Type tag to avoid dynamic_cast of contained QWidget*
 *
 * @value GenericType
 * @value FileTransferWidgetType = 0
 */

ChatLineContentProxy::ChatLineContentProxy(QWidget* widget, ChatLineContentProxyType type,
                                           int minWidth, float widthInPercent)
        : widthPercent(widthInPercent), widthMin(minWidth), widgetType{type} {
    proxy = new QGraphicsProxyWidget(this);
    proxy->setWidget(widget);
}

ChatLineContentProxy::ChatLineContentProxy(QWidget* widget, int minWidth, float widthInPercent)
        : ChatLineContentProxy(widget, GenericType, minWidth, widthInPercent) {}

ChatLineContentProxy::ChatLineContentProxy(FileTransferWidget* widget, int minWidth,
                                           float widthInPercent)
        : ChatLineContentProxy(widget, FileTransferWidgetType, minWidth, widthInPercent) {}

QRectF ChatLineContentProxy::boundingRect() const {
    QRectF result = proxy->boundingRect();
    result.setHeight(result.height() + 5);
    return result;
}

void ChatLineContentProxy::paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                                 QWidget* widget) {
    painter->setClipRect(boundingRect());
    proxy->paint(painter, option, widget);
}

qreal ChatLineContentProxy::getAscent() const { return 7.0; }

QWidget* ChatLineContentProxy::getWidget() const { return proxy->widget(); }

void ChatLineContentProxy::setWidth(qreal width) {
    prepareGeometryChange();
    proxy->widget()->setFixedWidth(qMax(static_cast<int>(width * widthPercent), widthMin));
}

ChatLineContentProxy::ChatLineContentProxyType ChatLineContentProxy::getWidgetType() const {
    return widgetType;
}
