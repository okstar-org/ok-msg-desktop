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

#ifndef CHATLINE_H
#define CHATLINE_H

#include <QDateTime>
#include <QPixmap>
#include <QPointF>
#include <QRectF>
#include <QVector>
#include <memory>
#include "src/persistence/history.h"

class QGraphicsScene;
class QStyleOptionGraphicsItem;
class QFont;

namespace module::im {
class ChatLog;
class ChatLineContent;

enum class IChatItemType { TEXT, IMAGE };

class IChatItem : public QObject {
    Q_OBJECT
public:
    using Ptr = std::shared_ptr<IChatItem>;

    explicit IChatItem(const MsgId& id = "");
    virtual ~IChatItem() = default;

    const MsgId& getId() { return id; }

    virtual IChatItemType itemType() = 0;
    virtual void layout(qreal width, QPointF scenePos) = 0;
    virtual QRectF sceneBoundingRect() const = 0;
    virtual void moveBy(qreal dx, qreal dy);
    virtual void addToScene(QGraphicsScene* scene);
    virtual void removeFromScene();
    virtual void setVisible(bool visible);

    virtual void markAsDelivered(const QDateTime& time) {};

    virtual ChatLineContent* contentAtPos(QPointF scenePos) const { return nullptr; }
    virtual ChatLineContent* centerContent() const;

    virtual bool selectable() const { return false; }

    virtual void visibilityChanged(bool visible);
    virtual void selectionFocusChanged(bool focusIn);
    virtual void reloadTheme();

public:
    void fontChanged(const QFont& font);
    void selectionCleared();
    void selectAll();
    void setTime(const QDateTime& time) { datetime = time; }
    QDateTime getTime() { return datetime; }

    void setRow(int row);
    int getRow() { return row; }
    bool isValid() { return row < 0; }

protected:
    friend class ChatLog;
    virtual QList<ChatLineContent*> contents() { return QList<ChatLineContent*>{}; };
    QDateTime datetime;
    int row = -1;
    MsgId id;
    ChatLog* chatLog;

signals:
    // 引用事件
    void replyEvent(IChatItem*);
};
}  // namespace module::im
#endif  // CHATLINE_H
