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

#ifndef CHATLINECONTENT_H
#define CHATLINECONTENT_H

#include <QGraphicsItem>

class ChatLineContent : public QObject, public QGraphicsItem {
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

public:
    enum GraphicsItemType {
        ChatLineContentType = QGraphicsItem::UserType + 1,
    };

    int getColumn() const;
    int getRow() const;

    virtual void setWidth(qreal width) = 0;
    virtual int type() const final;

    virtual void selectionMouseMove(QPointF scenePos);
    virtual void selectionStarted(QPointF scenePos);
    virtual void selectionCleared();
    virtual void selectionDoubleClick(QPointF scenePos);
    virtual void selectionTripleClick(QPointF scenePos);
    virtual void selectionFocusChanged(bool focusIn);
    virtual void selectAll();
    virtual bool isOverSelection(QPointF scenePos) const;
    virtual QString getSelectedText() const;
    virtual void fontChanged(const QFont& font);

    virtual QString getText() const;

    virtual qreal getAscent() const;

    virtual QRectF boundingRect() const = 0;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                       QWidget* widget) = 0;

    virtual void visibilityChanged(bool visible);
    virtual void reloadTheme();

private:
    friend class IChatItem;
    void setIndex(int row, int col);

private:
    int row = -1;
    int col = -1;
};

#endif  // CHATLINECONTENT_H
