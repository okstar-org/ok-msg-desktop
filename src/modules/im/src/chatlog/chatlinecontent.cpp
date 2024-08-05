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

#include "chatlinecontent.h"

void ChatLineContent::setIndex(int r, int c) {
    row = r;
    col = c;
}

int ChatLineContent::getColumn() const { return col; }

int ChatLineContent::getRow() const { return row; }

int ChatLineContent::type() const { return GraphicsItemType::ChatLineContentType; }

void ChatLineContent::selectionMouseMove(QPointF) {}

void ChatLineContent::selectionStarted(QPointF) {}

void ChatLineContent::selectionCleared() {}

void ChatLineContent::selectionDoubleClick(QPointF) {}

void ChatLineContent::selectionTripleClick(QPointF) {}

void ChatLineContent::selectionFocusChanged(bool) {}

void ChatLineContent::selectAll() {}

bool ChatLineContent::isOverSelection(QPointF) const { return false; }

QString ChatLineContent::getSelectedText() const { return QString(); }

void ChatLineContent::fontChanged(const QFont& font) { Q_UNUSED(font); }

qreal ChatLineContent::getAscent() const { return 0.0; }

void ChatLineContent::visibilityChanged(bool) {}

void ChatLineContent::reloadTheme() {}

QString ChatLineContent::getText() const { return QString(); }
