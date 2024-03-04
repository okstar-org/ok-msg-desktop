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

#include "chatline.h"
#include "chatlinecontent.h"

#include <QDebug>
#include <QGraphicsScene>

ChatLine::ChatLine()
{
}

ChatLine::~ChatLine()
{
    for (ChatLineContent* c : content) {
        if (c->scene())
            c->scene()->removeItem(c);

        delete c;
    }
}

void ChatLine::setRow(int idx)
{
    row = idx;

    for (int c = 0; c < static_cast<int>(content.size()); ++c)
        content[c]->setIndex(row, c);
}

void ChatLine::visibilityChanged(bool visible)
{
    if (isVisible != visible) {
        for (ChatLineContent* c : content)
            c->visibilityChanged(visible);
    }

    isVisible = visible;
}

int ChatLine::getRow() const
{
    return row;
}

ChatLineContent* ChatLine::getContent(int col) const
{
    if (col < static_cast<int>(content.size()) && col >= 0)
        return content[col];

    return nullptr;
}

ChatLineContent* ChatLine::getContent(QPointF scenePos) const
{
    for (ChatLineContent* c : content) {
        if (c->sceneBoundingRect().contains(scenePos))
            return c;
    }

    return nullptr;
}

void ChatLine::removeFromScene()
{
    for (ChatLineContent* c : content) {
        if (c->scene())
            c->scene()->removeItem(c);
    }
}

void ChatLine::addToScene(QGraphicsScene* scene)
{
    if (!scene)
        return;

    for (ChatLineContent* c : content)
        scene->addItem(c);
}

void ChatLine::setVisible(bool visible)
{
    for (ChatLineContent* c : content)
        c->setVisible(visible);
}

void ChatLine::selectionCleared()
{
    for (ChatLineContent* c : content)
        c->selectionCleared();
}

void ChatLine::selectionFocusChanged(bool focusIn)
{
    for (ChatLineContent* c : content)
        c->selectionFocusChanged(focusIn);
}

void ChatLine::fontChanged(const QFont& font)
{
    for (ChatLineContent* c : content)
        c->fontChanged(font);
}

void ChatLine::reloadTheme()
{
    for (ChatLineContent* c : content) {
        c->reloadTheme();
    }
}

int ChatLine::getColumnCount()
{
    return content.size();
}

void ChatLine::updateBBox()
{
    bbox.setHeight(0);
    bbox.setWidth(width);

    for (ChatLineContent* c : content)
        bbox.setHeight(qMax(c->sceneBoundingRect().top() - bbox.top() + c->sceneBoundingRect().height(),
                            bbox.height()));
}

QRectF ChatLine::sceneBoundingRect() const
{
    return bbox;
}

void ChatLine::addColumn(ChatLineContent* item, ColumnFormat fmt)
{
    if (!item)
        return;

    format.push_back(fmt);
    content.push_back(item);
}

void ChatLine::replaceContent(int col, ChatLineContent* lineContent)
{
    if (col >= 0 && col < static_cast<int>(content.size()) && lineContent) {
        QGraphicsScene* scene = content[col]->scene();
        delete content[col];

        content[col] = lineContent;
        lineContent->setIndex(row, col);

        if (scene)
            scene->addItem(content[col]);

        layout(width, bbox.topLeft());
        content[col]->visibilityChanged(isVisible);
        content[col]->update();
    }
}

void ChatLine::layout(qreal w, QPointF scenePos)
{
    if (!content.size())
        return;

    width = w;
    bbox.setTopLeft(scenePos);

    qreal fixedWidth = (content.size() - 1) * columnSpacing;
    qreal varWidth = 0.0; // used for normalisation

    for (int i = 0; i < format.size(); ++i) {
        if (format[i].policy == ColumnFormat::FixedSize)
            fixedWidth += format[i].size;
        else
            varWidth += format[i].size;
    }

    if (varWidth == 0.0)
        varWidth = 1.0;

    qreal leftover = qMax(0.0, width - fixedWidth);

    qreal maxVOffset = 0.0;
    qreal xOffset = 0.0;
    QVector<qreal> xPos(content.size());

    for (int i = 0; i < content.size(); ++i) {
        // calculate the effective width of the current column
        qreal width;
        if (format[i].policy == ColumnFormat::FixedSize)
            width = format[i].size;
        else
            width = format[i].size / varWidth * leftover;

        // set the width of the current column
        content[i]->setWidth(width);

        // calculate horizontal alignment
        qreal xAlign = 0.0;

        switch (format[i].hAlign) {
        case ColumnFormat::Left:
            break;
        case ColumnFormat::Right:
            xAlign = width - content[i]->boundingRect().width();
            break;
        case ColumnFormat::Center:
            xAlign = (width - content[i]->boundingRect().width()) / 2.0;
            break;
        }

        // reposition
        xPos[i] = scenePos.x() + xOffset + xAlign;

        xOffset += width + columnSpacing;
        maxVOffset = qMax(maxVOffset, content[i]->getAscent());
    }

    for (int i = 0; i < content.size(); ++i) {
        // calculate vertical alignment
        // vertical alignment may depend on width, so we do it in a second pass
        qreal yOffset = maxVOffset - content[i]->getAscent();

        // reposition
        content[i]->setPos(xPos[i], scenePos.y() + yOffset);
    }

    updateBBox();
}

void ChatLine::moveBy(qreal deltaY)
{
    // reposition only
    for (ChatLineContent* c : content)
        c->moveBy(0, deltaY);

    bbox.moveTop(bbox.top() + deltaY);
}

bool ChatLine::lessThanBSRectTop(const ChatLine::Ptr& lhs, const qreal& rhs)
{
    return lhs->sceneBoundingRect().top() < rhs;
}

bool ChatLine::lessThanBSRectBottom(const ChatLine::Ptr& lhs, const qreal& rhs)
{
    return lhs->sceneBoundingRect().bottom() < rhs;
}

bool ChatLine::lessThanRowIndex(const ChatLine::Ptr& lhs, const ChatLine::Ptr& rhs)
{
    return lhs->getRow() < rhs->getRow();
}
