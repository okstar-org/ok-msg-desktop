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

#include "chattextedit.h"

#include "UI/widget/im/translator.h"

#include <QApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QMimeData>

ChatTextEdit::ChatTextEdit(QWidget* parent) : QTextEdit(parent) {
    retranslateUi();
    setAcceptRichText(false);
    setAcceptDrops(false);

    settings::Translator::registerHandler(std::bind(&ChatTextEdit::retranslateUi, this), this);
}

ChatTextEdit::~ChatTextEdit() { Translator::unregister(this); }

void ChatTextEdit::keyPressEvent(QKeyEvent* event) {
    int key = event->key();
    if ((key == Qt::Key_Enter || key == Qt::Key_Return) &&
        !(event->modifiers() & Qt::ShiftModifier)) {
        emit enterPressed();
        return;
    }
    if (key == Qt::Key_Tab) {
        if (event->modifiers())
            event->ignore();
        else {
            emit tabPressed();
            event->ignore();
        }
        return;
    }
    if (key == Qt::Key_Up && this->toPlainText().isEmpty()) {
        this->setPlainText(lastMessage);
        this->setFocus();
        this->moveCursor(QTextCursor::MoveOperation::End, QTextCursor::MoveMode::MoveAnchor);
        return;
    }
    if (event->matches(QKeySequence::Paste) && pasteIfImage(event)) {
        return;
    }
    emit keyPressed();
    QTextEdit::keyPressEvent(event);
}

void ChatTextEdit::setLastMessage(QString lm) { lastMessage = lm; }

void ChatTextEdit::retranslateUi() { setPlaceholderText(tr("Type your message here...")); }

void ChatTextEdit::sendKeyEvent(QKeyEvent* event) { emit keyPressEvent(event); }

bool ChatTextEdit::pasteIfImage(QKeyEvent* event) {
    const QClipboard* const clipboard = QApplication::clipboard();
    if (!clipboard) {
        return false;
    }

    const QMimeData* const mimeData = clipboard->mimeData();
    if (!mimeData || !mimeData->hasImage()) {
        return false;
    }

    const QPixmap pixmap(clipboard->pixmap());
    if (pixmap.isNull()) {
        return false;
    }

    emit pasteImage(pixmap);
    return true;
}
