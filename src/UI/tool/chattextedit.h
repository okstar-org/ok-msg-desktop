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

#ifndef CHATTEXTEDIT_H
#define CHATTEXTEDIT_H

#include <QTextEdit>

class ChatTextEdit final : public QTextEdit {
    Q_OBJECT
public:
    explicit ChatTextEdit(QWidget* parent = nullptr);
    ~ChatTextEdit();
    void setLastMessage(QString lm);
    void sendKeyEvent(QKeyEvent* event);

signals:
    void enterPressed();
    void tabPressed();
    void keyPressed();
    void pasteImage(const QPixmap& pixmap);

protected:
    virtual void keyPressEvent(QKeyEvent* event) final override;

private:
    void retranslateUi();
    bool pasteIfImage(QKeyEvent* event);

private:
    QString lastMessage;
};

#endif  // CHATTEXTEDIT_H
