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

class ChatForm : public GenericChatForm {
  Q_OBJECT
public:
  ChatForm(const ToxPk *contact,
           IChatLog &chatLog,
           IMessageDispatcher &messageDispatcher);
  ~ChatForm();
  void setStatusMessage(const QString &newMessage);

  void setFriendTyping(bool isTyping);

  virtual void show(ContentLayout *contentLayout) final override;
  virtual void reloadTheme() final override;

  static const QString ACTION_PREFIX;

signals:

  void incomingNotification(QString friendId);
  void outgoingNotification();
  void stopNotification();
  void endCallNotification();
  void rejectCall(QString friendId);
  void acceptCall(QString friendId);
  void updateFriendActivity(const ToxPk &frnd);

public slots:
  void onAvInvite(QString friendId, bool video);
  void onAvStart(QString friendId, bool video);
  void onAvEnd(QString friendId, bool error);
  void onFileNameChanged(const ToxPk &friendPk);
  void clearChatArea();

private slots:
  void updateFriendActivityForFile(const ToxFile &file);
  void onAttachClicked() override;
  void onScreenshotClicked() override;

  void onTextEditChanged();
  void onCallTriggered();
  void onVideoCallTriggered();
  void onAnswerCallTriggered(bool video);
  void onRejectCallTriggered();
  void onMicMuteToggle();
  void onVolMuteToggle();

  void onFriendStatusChanged(const ToxPk& friendId, Status::Status status);
  void onFriendTypingChanged(const ToxPk& friendId, bool isTyping);
  void onFriendNameChanged(const QString &name);
  void onStatusMessage(const QString &message);
  void onUpdateTime();
  void sendImage(const QPixmap &pixmap);
  void doScreenshot();
  void onCopyStatusMessage();

  void callUpdateFriendActivity();

private:

  void retranslateUi();
  void showOutgoingCall(bool video);
  void startCounter();
  void stopCounter(bool error = false);


protected:
  GenericNetCamView *createNetcam() final override;
  void insertChatMessage(IChatItem::Ptr msg) final override;
  void dragEnterEvent(QDragEnterEvent *ev) final override;
  void dropEvent(QDropEvent *ev) final override;
  void hideEvent(QHideEvent *event) final override;
  void showEvent(QShowEvent *event) final override;

private:
  const ToxPk *f;
  CroppingLabel *statusMessageLabel;
  QMenu statusMessageMenu;
  QLabel *callDuration;
  QTimer *callDurationTimer;
  QTimer typingTimer;
  QElapsedTimer timeElapsed;
  QAction *copyStatusAction;
  bool isTyping;
  bool lastCallIsVideo;
};

#endif // CHATFORM_H
