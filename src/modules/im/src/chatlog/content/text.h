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

#ifndef TEXT_H
#define TEXT_H

#include "../chatlinecontent.h"
#include "src/lib/settings/style.h"

#include <QFont>

class QTextDocument;

class Text : public ChatLineContent {
    Q_OBJECT

public:
    Text(const QString& txt = "", const QFont& font = QFont(), bool enableElide = false,
         const QString& rawText = QString());
    virtual ~Text();

    void setTextSelectable(bool selectable);
    void setText(const QString& txt);
    void selectText(const QString& txt, const std::pair<int, int>& point);
    void selectText(const QRegularExpression& exp, const std::pair<int, int>& point);
    void deselectText();

    virtual void setWidth(qreal width) final;

    virtual void selectionMouseMove(QPointF scenePos) final;
    virtual void selectionStarted(QPointF scenePos) final;
    virtual void selectionCleared() final;
    virtual void selectionDoubleClick(QPointF scenePos) final;
    virtual void selectionTripleClick(QPointF scenePos) final;
    virtual void selectionFocusChanged(bool focusIn) final;
    virtual void selectAll() final;
    virtual bool isOverSelection(QPointF scenePos) const final;
    virtual QString getSelectedText() const final;
    virtual void fontChanged(const QFont& font) final;

    virtual QRectF boundingRect() const final;
    virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
                       QWidget* widget) final;

    virtual void visibilityChanged(bool keepInMemory) final;
    virtual void reloadTheme() final override;

    virtual qreal getAscent() const final;
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* event) final override;
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) final override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* event) final override;

    virtual QString getText() const final;
    QString getLinkAt(QPointF scenePos) const;

    void setContentsMargins(QMarginsF margins);

    void setBoundingRadius(qreal radius);
    void setBackgroundColor(const QColor& color);
    void setColor(Style::ColorPalette role);
    void setColor(const QColor& color);

protected:
    // dynamic resource management
    void regenerate();
    void freeResources();

    virtual QSizeF idealSize();
    int cursorFromPos(QPointF scenePos, bool fuzzy = true) const;
    int getSelectionEnd() const;
    int getSelectionStart() const;
    bool hasSelection() const;
    QString extractSanitizedText(int from, int to) const;
    QString extractImgTooltip(int pos) const;

    QTextDocument* doc = nullptr;
    QSizeF size;
    qreal width = 0.0;

private:
    void selectText(QTextCursor& cursor, const std::pair<int, int>& point);
    QColor textColor() const;

    QString text;
    QString rawText;
    QString selectedText;
    bool keepInMemory = false;
    bool elide = false;
    bool dirty = false;
    bool selectionHasFocus = true;
    int selectionEnd = -1;
    int selectionAnchor = -1;
    qreal ascent = 0.0;
    QFont defFont;
    QString defStyleSheet;

    QColor backgroundColor;
    bool isCustomColor = false;
    Style::ColorPalette colorRole = Style::MainText;
    QColor color;

    qreal boundRadius = 0.0;
    QMarginsF margins;
    bool selectable = true;
};

#endif  // TEXT_H
