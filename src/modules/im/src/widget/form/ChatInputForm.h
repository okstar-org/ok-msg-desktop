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

//
// Created by gaojie on 24-9-29.
//
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include <QWidget>

class QHBoxLayout;
class QVBoxLayout;
class QFile;

class EmoticonsWidget;
class ChatTextEdit;
class IChatItem;
class ChatReplyForm;
/**
 * 聊天输入框
 */
class ChatInputForm : public QWidget {
    Q_OBJECT
public:
    ChatInputForm(QWidget* parent = nullptr, bool isGroup = false);
    void reloadTheme();
    void retranslateUi();
    void setFocus();
    void updateFont(const QFont& font);
    QString getInputText();

    void insertReplyText(const QString& id, QString nickname, QString content);

protected:
    virtual void keyPressEvent(QKeyEvent* ev) final override;
    virtual void keyReleaseEvent(QKeyEvent* ev) final override;
    // drag & drop
    virtual void dragEnterEvent(QDragEnterEvent* ev) final override;
    virtual void dropEvent(QDropEvent* ev) final override;

private:
    bool isEncrypt;
    bool isGroup;

    QVBoxLayout* mainLayout;
    QHBoxLayout* mainFootLayout;
    QHBoxLayout* sendLayout;

    QPushButton* encryptButton;
    QPushButton* emoteButton;
    QPushButton* fileButton;
    QPushButton* screenshotButton;
    QPushButton* sendButton;
    ChatTextEdit* msgEdit;

    ChatReplyForm* reply;

    EmoticonsWidget* emoticonsWidget;

    void doScreenshot();

protected slots:
#ifdef OK_PLUGIN
    void onPluginEnabled(const QString& shortName);
    void onPluginDisabled(const QString& shortName);
#endif
    void onEncryptButtonClicked();
    void onEmoteButtonClicked();
    void onEmoteInsertRequested(QString str);

    void onSendTriggered();
    void onAttachClicked();
    void onScreenshotClicked();
    void onScreenCaptured(const QPixmap& pixmap);
    void onTextEditChanged();
    void onReplyEvent(IChatItem* item);
    void onReplyRemove();

signals:
    void inputText(const QString& text);
    void inputTextChanged(const QString& text);
    void inputFile(const QFile& file);
    void inputScreenCapture(const QPixmap& pixmap);
};
