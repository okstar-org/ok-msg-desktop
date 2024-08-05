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

#include "text.h"
#include "../documentcache.h"

#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QDebug>
#include <QDesktopServices>
#include <QFontMetrics>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QTextBlock>
#include <QTextFragment>

Text::Text(const QString& txt, const QFont& font, bool enableElide, const QString& rwText)
        : rawText(rwText)
        , elide(enableElide)
        , defFont(font)
        , defStyleSheet(Style::getStylesheet(QStringLiteral("chatArea/innerStyle.css"), font)) {
    color = textColor();
    setText(txt);
    setAcceptedMouseButtons(Qt::LeftButton);
    setAcceptHoverEvents(true);
}

Text::~Text() {
    if (doc) DocumentCache::getInstance().push(doc);
}

void Text::setTextSelectable(bool selectable) { this->selectable = selectable; }

void Text::setText(const QString& txt) {
    text = txt;
    dirty = true;
}

void Text::selectText(const QString& txt, const std::pair<int, int>& point) {
    regenerate();

    if (!doc || !selectable) {
        return;
    }

    auto cursor = doc->find(txt, point.first);

    selectText(cursor, point);
}

void Text::selectText(const QRegularExpression& exp, const std::pair<int, int>& point) {
    regenerate();

    if (!doc || !selectable) {
        return;
    }

    auto cursor = doc->find(exp, point.first);

    selectText(cursor, point);
}

void Text::deselectText() {
    dirty = true;
    regenerate();
    update();
}

void Text::setWidth(qreal w) {
    width = w;
    dirty = true;

    regenerate();
}

void Text::selectionMouseMove(QPointF scenePos) {
    if (!doc || !selectable) return;

    int cur = cursorFromPos(scenePos);
    if (cur >= 0) {
        selectionEnd = cur;
        selectedText = extractSanitizedText(getSelectionStart(), getSelectionEnd());
    }

    update();
}

void Text::selectionStarted(QPointF scenePos) {
    int cur = cursorFromPos(scenePos);
    if (cur >= 0) {
        selectionEnd = cur;
        selectionAnchor = cur;
    }
}

void Text::selectionCleared() {
    selectedText.clear();
    selectedText.squeeze();

    // Do not reset selectionAnchor!
    selectionEnd = -1;

    update();
}

void Text::selectionDoubleClick(QPointF scenePos) {
    if (!doc) return;

    int cur = cursorFromPos(scenePos);

    if (cur >= 0) {
        QTextCursor cursor(doc);
        cursor.setPosition(cur);
        cursor.select(QTextCursor::WordUnderCursor);

        selectionAnchor = cursor.selectionStart();
        selectionEnd = cursor.selectionEnd();

        selectedText = extractSanitizedText(getSelectionStart(), getSelectionEnd());
    }

    update();
}

void Text::selectionTripleClick(QPointF scenePos) {
    if (!doc) return;

    int cur = cursorFromPos(scenePos);

    if (cur >= 0) {
        QTextCursor cursor(doc);
        cursor.setPosition(cur);
        cursor.select(QTextCursor::BlockUnderCursor);

        selectionAnchor = cursor.selectionStart();
        selectionEnd = cursor.selectionEnd();

        if (cursor.block().isValid() && cursor.block().blockNumber() != 0) selectionAnchor++;

        selectedText = extractSanitizedText(getSelectionStart(), getSelectionEnd());
    }

    update();
}

void Text::selectionFocusChanged(bool focusIn) {
    selectionHasFocus = focusIn;
    update();
}

void Text::selectAll() {
    if (!doc) return;

    QTextCursor cursor(doc);
    cursor.setPosition(0);
    cursor.select(QTextCursor::Document);
    selectionAnchor = cursor.selectionStart();
    selectionEnd = cursor.selectionEnd();
    selectedText = text;
    update();
}

bool Text::isOverSelection(QPointF scenePos) const {
    int cur = cursorFromPos(scenePos);
    if (getSelectionStart() < cur && getSelectionEnd() >= cur) return true;

    return false;
}

QString Text::getSelectedText() const { return selectedText; }

void Text::fontChanged(const QFont& font) { defFont = font; }

QRectF Text::boundingRect() const {
    return QRectF(QPointF(0, 0),
                  size + QSize(margins.left() + margins.right(), margins.top() + margins.bottom()));
}

void Text::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) {
    Q_UNUSED(option);
    Q_UNUSED(widget);

    if (!doc) return;

    painter->setRenderHint(QPainter::Antialiasing);
    if (backgroundColor.isValid()) {
        painter->save();
        painter->translate(0.5, 0.5);
        if (boundRadius > 0) {
            QPainterPath path;
            path.addRoundedRect(boundingRect(), boundRadius, boundRadius);
            painter->fillPath(path, backgroundColor);
        } else {
            painter->fillRect(boundingRect(), backgroundColor);
        }
        painter->restore();
    }
    painter->save();
    painter->setClipRect(boundingRect());

    // draw selection
    QAbstractTextDocumentLayout::PaintContext ctx;
    QAbstractTextDocumentLayout::Selection sel;

    if (hasSelection()) {
        sel.cursor = QTextCursor(doc);
        sel.cursor.setPosition(getSelectionStart());
        sel.cursor.setPosition(getSelectionEnd(), QTextCursor::KeepAnchor);
    }

    const QColor selectionColor = Style::getColor(Style::SelectText);
    sel.format.setBackground(selectionColor.lighter(selectionHasFocus ? 100 : 160));
    sel.format.setForeground(selectionHasFocus ? Qt::white : Qt::black);

    ctx.selections.append(sel);
    ctx.palette.setColor(QPalette::Text, color);

    // draw text
    painter->translate(margins.left(), margins.top());
    doc->documentLayout()->draw(painter, ctx);
    painter->restore();
}

void Text::visibilityChanged(bool visible) {
    keepInMemory = visible;

    regenerate();
    update();
}

void Text::reloadTheme() {
    defStyleSheet = Style::getStylesheet(QStringLiteral("chatArea/innerStyle.css"), defFont);
    color = textColor();
    dirty = true;
    regenerate();
    update();
}

qreal Text::getAscent() const { return ascent; }

void Text::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) event->accept();  // grabber
}

void Text::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    if (!doc) return;

    QString anchor = doc->documentLayout()->anchorAt(event->pos());

    // open anchor in browser
    if (!anchor.isEmpty()) QDesktopServices::openUrl(anchor);
}

void Text::hoverMoveEvent(QGraphicsSceneHoverEvent* event) {
    if (!doc) return;

    QString anchor = doc->documentLayout()->anchorAt(event->pos());

    if (anchor.isEmpty())
        setCursor(selectable ? Qt::IBeamCursor : Qt::ArrowCursor);
    else
        setCursor(Qt::PointingHandCursor);

    // tooltip
    setToolTip(extractImgTooltip(cursorFromPos(event->scenePos(), false)));
}

QString Text::getText() const { return rawText; }

/**
 * @brief Extracts the target of a link from the text at a given coordinate
 * @param scenePos Position in scene coordinates
 * @return The link target URL, or an empty string if there is no link there
 */
QString Text::getLinkAt(QPointF scenePos) const {
    QTextCursor cursor(doc);
    cursor.setPosition(cursorFromPos(scenePos));
    return cursor.charFormat().anchorHref();
}

void Text::setContentsMargins(QMarginsF margins) { this->margins = margins; }

void Text::setBoundingRadius(qreal radius) {
    if (boundRadius != radius) {
        boundRadius = radius;
        update();
    }
}

void Text::setBackgroundColor(const QColor& color) {
    if (backgroundColor != color) {
        backgroundColor = color;
        update();
    }
}

void Text::setColor(Style::ColorPalette role) {
    if (isCustomColor || colorRole != role) {
        isCustomColor = false;
        colorRole = role;
        color = textColor();
        update();
    }
}

void Text::setColor(const QColor& color) {
    if (!isCustomColor || this->color != color) {
        isCustomColor = true;
        this->color = color;
        update();
    }
}

void Text::regenerate() {
    if (!doc) {
        doc = DocumentCache::getInstance().pop();
        dirty = true;
    }

    if (dirty) {
        doc->setDefaultFont(defFont);

        if (elide) {
            QFontMetrics metrics = QFontMetrics(defFont);
            QString elidedText = metrics.elidedText(text, Qt::ElideRight, qRound(width));

            doc->setPlainText(elidedText);
        } else {
            doc->setDefaultStyleSheet(defStyleSheet);
            doc->setHtml(text);
        }

        // wrap mode
        QTextOption opt;
        opt.setWrapMode(elide ? QTextOption::NoWrap : QTextOption::WrapAtWordBoundaryOrAnywhere);
        doc->setDefaultTextOption(opt);

        // width
        doc->setTextWidth(width);
        doc->documentLayout()->update();

        // update ascent
        if (doc->firstBlock().layout()->lineCount() > 0)
            ascent = doc->firstBlock().layout()->lineAt(0).ascent();

        // let the scene know about our change in size
        if (size != idealSize()) prepareGeometryChange();

        // get the new width and height
        size = idealSize();

        dirty = false;
    }

    // if we are not visible -> free mem
    if (!keepInMemory) freeResources();
}

void Text::freeResources() {
    DocumentCache::getInstance().push(doc);
    doc = nullptr;
}

QSizeF Text::idealSize() {
    if (doc) return doc->size();

    return size;
}

int Text::cursorFromPos(QPointF scenePos, bool fuzzy) const {
    if (doc) {
        QPointF pos = mapFromScene(scenePos);
        QRectF rect = this->boundingRect();
        if (rect.contains(pos)) {
            pos.ry() = qBound(
                    rect.top() + margins.top() + 1, pos.y(), rect.bottom() - margins.bottom() - 1);
        }
        return doc->documentLayout()->hitTest(pos, fuzzy ? Qt::FuzzyHit : Qt::ExactHit);
    }
    return -1;
}

int Text::getSelectionEnd() const { return qMax(selectionAnchor, selectionEnd); }

int Text::getSelectionStart() const { return qMin(selectionAnchor, selectionEnd); }

bool Text::hasSelection() const { return selectionEnd >= 0; }

QString Text::extractSanitizedText(int from, int to) const {
    if (!doc) return "";

    QString txt;

    QTextBlock begin = doc->findBlock(from);
    QTextBlock end = doc->findBlock(to);
    for (QTextBlock block = begin; block != end.next() && block.isValid(); block = block.next()) {
        for (QTextBlock::Iterator itr = block.begin(); itr != block.end(); ++itr) {
            int pos = itr.fragment().position();  // fragment position -> position of the first
                                                  // character in the fragment

            if (itr.fragment().charFormat().isImageFormat()) {
                QTextImageFormat imgFmt = itr.fragment().charFormat().toImageFormat();
                QString key = imgFmt.name();  // img key (eg. key::D for :D)
                QString rune = key.mid(4);

                if (pos >= from && pos < to) {
                    txt += rune;
                    ++pos;
                }
            } else {
                for (QChar c : itr.fragment().text()) {
                    if (pos >= from && pos < to) txt += c;

                    ++pos;
                }
            }
        }

        txt += '\n';
    }

    txt.chop(1);

    return txt;
}

QString Text::extractImgTooltip(int pos) const {
    for (QTextBlock::Iterator itr = doc->firstBlock().begin(); itr != doc->firstBlock().end();
         ++itr) {
        if (itr.fragment().contains(pos) && itr.fragment().charFormat().isImageFormat()) {
            QTextImageFormat imgFmt = itr.fragment().charFormat().toImageFormat();
            return imgFmt.toolTip();
        }
    }

    return QString();
}

void Text::selectText(QTextCursor& cursor, const std::pair<int, int>& point) {
    if (!cursor.isNull()) {
        cursor.beginEditBlock();
        cursor.setPosition(point.first);
        cursor.setPosition(point.first + point.second, QTextCursor::KeepAnchor);
        cursor.endEditBlock();

        QTextCharFormat format;
        format.setBackground(QBrush(Style::getColor(Style::SearchHighlighted)));
        cursor.mergeCharFormat(format);

        regenerate();
        update();
    }
}

QColor Text::textColor() const { return isCustomColor ? color : Style::getColor(colorRole); }