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

#include <QPointF>
#include <QRectF>
#include <QVector>
#include <memory>

class ChatLog;
class ChatLineContent;
class QGraphicsScene;
class QStyleOptionGraphicsItem;
class QFont;

struct ColumnFormat
{
    enum Policy
    {
        FixedSize,
        VariableSize,
    };

    enum Align
    {
        Left,
        Center,
        Right,
    };

    ColumnFormat()
    {
    }
    ColumnFormat(qreal s, Policy p, Align halign = Left)
        : size(s)
        , policy(p)
        , hAlign(halign)
    {
    }

    qreal size = 1.0;
    Policy policy = VariableSize;
    Align hAlign = Left;
};

using ColumnFormats = QVector<ColumnFormat>;

class ChatLine
{
public:
    using Ptr = std::shared_ptr<ChatLine>;

    ChatLine();
    virtual ~ChatLine();

    QRectF sceneBoundingRect() const;

    void replaceContent(int col, ChatLineContent* lineContent);
    void layout(qreal width, QPointF scenePos);
    void moveBy(qreal deltaY);
    void removeFromScene();
    void addToScene(QGraphicsScene* scene);
    void setVisible(bool visible);
    void selectionCleared();
    void selectionFocusChanged(bool focusIn);
    void fontChanged(const QFont& font);
    void reloadTheme();

    int getColumnCount();
    int getRow() const;

    ChatLineContent* getContent(int col) const;
    ChatLineContent* getContent(QPointF scenePos) const;

    bool isOverSelection(QPointF scenePos);

    // comparators
    static bool lessThanBSRectTop(const ChatLine::Ptr& lhs, const qreal& rhs);
    static bool lessThanBSRectBottom(const ChatLine::Ptr& lhs, const qreal& rhs);
    static bool lessThanRowIndex(const ChatLine::Ptr& lhs, const ChatLine::Ptr& rhs);

protected:
    friend class ChatLog;

    QPointF mapToContent(ChatLineContent* c, QPointF pos);

    void addColumn(ChatLineContent* item, ColumnFormat fmt);
    void updateBBox();
    void setRow(int idx);
    void visibilityChanged(bool visible);

private:
    int row = -1;
    QVector<ChatLineContent*> content;
    QVector<ColumnFormat> format;
    qreal width = 100.0;
    qreal columnSpacing = 15.0;
    QRectF bbox;
    bool isVisible = false;
};

#endif // CHATLINE_H
