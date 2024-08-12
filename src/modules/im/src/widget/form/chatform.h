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

#ifndef CHATFORM_H
#define CHATFORM_H

#include <QElapsedTimer>
#include <QLabel>
#include <QSet>
#include <QTimer>

#include "CallDurationForm.h"
#include "genericchatform.h"
#include "src/core/core.h"
#include "src/model/ichatlog.h"
#include "src/model/imessagedispatcher.h"
#include "src/model/status.h"
#include "src/persistence/history.h"
#include "src/widget/tool/screenshotgrabber.h"

class CallConfirmWidget;
class FileTransferInstance;
class Friend;
class History;
class OfflineMsgEngine;
class QPixmap;
class QHideEvent;
class QMoveEvent;

/**
 * 聊天框
 * @brief The ChatForm class
 */
class ChatForm : public GenericChatForm {
    Q_OBJECT
public:
    static const QString ACTION_PREFIX;

    ChatForm(const FriendId* contact, IChatLog& chatLog, IMessageDispatcher& messageDispatcher);
    ~ChatForm();

    void setStatusMessage(const QString& newMessage);

    void setFriendTyping(bool isTyping);

    virtual void show(ContentLayout* contentLayout) final override;

    virtual void reloadTheme() final override;

    void insertChatMessage(IChatItem::Ptr msg) final override;

signals:
    void incomingNotification(QString friendId);
    void outgoingNotification();
    void stopNotification();
    void endCallNotification();

    void updateFriendActivity(const FriendId& frnd);

public slots:

    void onFileNameChanged(const FriendId& friendPk);
    void clearChatArea();

private slots:
    void updateFriendActivityForFile(const ToxFile& file);
    void onAttachClicked() override;
    void onScreenshotClicked() override;

    void onTextEditChanged();
    //  void onCallTriggered();
    //  void onVideoCallTriggered();
    //  void onAcceptCallTriggered(const ToxPeer &peer, bool video);
    //  void onRejectCallTriggered(const ToxPeer &peer);
    //  void onMicMuteToggle();
    //  void onVolMuteToggle();

    void onFriendStatusChanged(const FriendId& friendId, Status::Status status);
    void onFriendNameChanged(const QString& name);
    void onStatusMessage(const QString& message);

    void sendImage(const QPixmap& pixmap);
    void doScreenshot();
    void onCopyStatusMessage();
    void callUpdateFriendActivity();

protected:
    void dragEnterEvent(QDragEnterEvent* ev) final override;
    void dropEvent(QDropEvent* ev) final override;
    void hideEvent(QHideEvent* event) final override;
    void showEvent(QShowEvent* event) final override;

private:
    void retranslateUi();
    void showOutgoingCall(bool video);

    const FriendId* f;
    CroppingLabel* statusMessageLabel;
    QMenu statusMessageMenu;

    QTimer typingTimer;
    QAction* copyStatusAction;
    bool isTyping;
};

#endif  // CHATFORM_H
