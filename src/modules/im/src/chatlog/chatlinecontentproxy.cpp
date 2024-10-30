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
namespace module::im {
/**
 * @enum ChatLineContentProxy::ChatLineContentProxyType
 * @brief Type tag to avoid dynamic_cast of contained QWidget*
 *
 * @value GenericType
 * @value FileTransferWidgetType = 0
 */

ChatLineContentProxy::ChatLineContentProxy(QWidget* widget, ChatLineContentProxyType type,
                                           int minWidth, float widthInPercent)
        : ChatLineContent(ContentType::CHAT_PROXY)
        , widthPercent(widthInPercent)
        , widthMin(minWidth)
        , widgetType{type} {
    proxy = new QGraphicsProxyWidget(this);
    proxy->setWidget(widget);
}

ChatLineContentProxy::ChatLineContentProxy(QWidget* widget, int minWidth, float widthInPercent)
        : ChatLineContentProxy(widget, GenericType, minWidth, widthInPercent) {}

ChatLineContentProxy::ChatLineContentProxy(FileTransferWidget* widget,
                                           int minWidth,
                                           float widthInPercent)
        : ChatLineContentProxy(widget, FileTransferWidgetType, minWidth, widthInPercent) {}

void ChatLineContentProxy::onCopyEvent() {
    qDebug() << __func__;
    auto ftw = dynamic_cast<FileTransferWidget*>(getWidget());
    if (ftw) {
        ftw->onCopy();
    }
}

QRectF ChatLineContentProxy::boundingRect() const {
    QRectF result = proxy->boundingRect();
    result.setHeight(result.height() + 5);
    return result;
}

void ChatLineContentProxy::paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*) {}

qreal ChatLineContentProxy::getAscent() const { return 7.0; }

QWidget* ChatLineContentProxy::getWidget() const { return proxy->widget(); }

void ChatLineContentProxy::setWidth(qreal width) {
    prepareGeometryChange();
    int w = qMax(static_cast<int>(width * widthPercent), widthMin);
    proxy->setGeometry(QRectF(0, 0, w, proxy->size().height()));
}

ChatLineContentProxy::ChatLineContentProxyType ChatLineContentProxy::getWidgetType() const {
    return widgetType;
}

const void* ChatLineContentProxy::getContent() { return proxy; }
}  // namespace module::im