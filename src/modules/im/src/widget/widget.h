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

#ifndef WIDGET_H
#define WIDGET_H

#include "ui_mainwindow.h"

#include <QFileInfo>
#include <QMainWindow>
#include <QPointer>
#include <QSystemTrayIcon>

#include "genericchatitemwidget.h"

#include "src/audio/iaudiocontrol.h"
#include "src/audio/iaudiosink.h"
#include "src/core/core.h"
#include "src/core/groupid.h"
#include "src/core/toxfile.h"
#include "src/core/toxid.h"
#include "src/core/toxpk.h"
#include "src/model/friendmessagedispatcher.h"
#include "src/model/groupmessagedispatcher.h"
#include "ui_mainwindow.h"
#if DESKTOP_NOTIFICATIONS
#include "src/platform/desktop_notifications/desktopnotify.h"
#endif


#define PIXELS_TO_ACT 7

namespace Ui {
class IMMainWindow;
}

class AddFriendForm;
class AlSink;
class Camera;
class ChatForm;
class CircleWidget;
class ContentDialog;
class ContentLayout;
class Core;
class FilesForm;
class Friend;
class FriendChatroom;
class FriendListWidget;
//class FriendWidget;
class GenericChatroomWidget;
class Group;
class GroupChatForm;
class GroupChatroom;
class GroupInvite;
class GroupInviteForm;
class GroupWidget;
class MaskablePixmapWidget;
class ProfileForm;
class ProfileInfo;
class QActionGroup;
class QMenu;
class QPushButton;
class QSplitter;
class QTimer;
class SettingsWidget;
class SystemTrayIcon;
class VideoSurface;
class UpdateCheck;
class Settings;
class IChatLog;
class ChatHistory;
class ChatWidget;
class ContactWidget;



enum class DialogType {
  AddDialog,
  TransferDialog,
  SettingDialog,
  ProfileDialog,
  GroupDialog
};

enum class ActiveToolMenuButton {

  AddButton,
  GroupButton,
  TransferButton,
  SettingButton,
  None,
};



class Widget final : public QFrame {
  Q_OBJECT

private:


public:
  explicit Widget(IAudioControl &audio, QWidget *parent = nullptr);
  ~Widget() override;

  static Widget *getInstance(IAudioControl *audio = nullptr);
  bool newMessageAlert(QWidget *currentWindow, bool isActive, bool sound = true,
                        bool notify = true);

  void init();
  void setCentralWidget(QWidget *widget, const QString &widgetName);
  QString getUsername();
  Camera *getCamera();

  void showUpdateDownloadProgress();
  void addFriendDialog(const Friend *frnd, ContentDialog *dialog);
  void addGroupDialog(const Group *group, ContentDialog *dialog);
  bool newFriendMessageAlert(const ToxPk &friendId, const QString &text,
                             bool sound = true, bool file = false);
  bool newGroupMessageAlert(const GroupId &groupId, const ToxPk &authorPk,
                            const QString &message, bool notify);
  bool getIsWindowMinimized();
  void updateIcons();

  static QString fromDialogType(DialogType type);
  ContentDialog *createContentDialog() const;
  ContentLayout *createContentDialog(DialogType type) const;

  static void confirmExecutableOpen(const QFileInfo &file);

  void clearAllReceipts();

  void reloadTheme();


  void resetIcon();

//  [[nodiscard]] ContentLayout *getContentLayout() const {
//    return contentLayout;
//  }



public slots:
  void onShowSettings();
  void onSeparateWindowClicked(bool separate);
  void onSeparateWindowChanged(bool separate, bool clicked);
  void setWindowTitle(const QString &title);
  void forceShow();
  void onConnected();
  void onDisconnected();
  void onStatusSet(Status::Status status);
  void onFailedToStartCore();
  void onBadProxyCore();
  void onSelfAvatarLoaded(const QPixmap &pic);

  void setUsername(const QString &username);

  void setAvatar( QByteArray avatar);
  void addFriend0(const ToxPk &friendPk);
  void addFriend(QString friendId, const ToxPk &friendPk, bool isFriend);
  void addFriendDone();
  void addFriendFailed(const ToxPk &userId, const QString &errorInfo = QString());


  void onFileReceiveRequested(const ToxFile &file);

  void onEmptyGroupCreated(QString groupnumber, const GroupId &groupId, const QString &title);


  void titleChangedByUser(const QString &title);
  void onGroupPeerAudioPlaying(QString groupnumber, ToxPk peerPk);

  void nextContact();
  void previousContact();
  void onFriendDialogShown(const Friend *f);
  void onGroupDialogShown(const Group *g);
  void toggleFullscreen();
  void refreshPeerListsLocal(const QString &username);
  void onUpdateAvailable();
  void onCoreChanged(Core &core);

signals:
  void friendRequestAccepted(const ToxPk &friendPk);
  void friendRequestRejected(const ToxPk &friendPk);
  void friendRequested(const ToxId &friendAddress, const QString &nick, const QString &message);


  void statusSet(Status::Status status);
  void statusSelected(Status::Status status);
  void usernameChanged(const QString &username);
  void changeGroupTitle(QString groupnumber, const QString &title);
  void statusMessageChanged(const QString &statusMessage);
  void resized();
  void windowStateChanged(Qt::WindowStates states);
  void toSendMessage(const QString &to, bool isGroup = false);
  void toShowDetails(const ContactId &to);
  void toDeleteChat(const QString &to);
  void toClearHistory(const QString &to);

private slots:

  void onTransferClicked();
  void showProfile();
  void openNewDialog(GenericChatroomWidget *widget);
  void onChatroomWidgetClicked(GenericChatroomWidget *widget);
  void onStatusMessageChanged(const QString &newStatusMessage);
  void removeFriend(const ToxPk &friendId);
  void addFriend1(const ToxPk &friendId);
  void copyFriendIdToClipboard(const ToxPk &friendId);
  void removeGroup(const GroupId &groupId);
  void destroyGroup(const GroupId &groupId);

  void onIconClick(QSystemTrayIcon::ActivationReason);
  void onUserAwayCheck();
  void onEventIconTick();
  void onTryCreateTrayIcon();
  void onSetShowSystemTray(bool newValue);
  void onSplitterMoved(int pos, int index);
  void friendListContextMenu(const QPoint &pos);


  void onDialogShown(GenericChatroomWidget *widget);
  void outgoingNotification();
  void onCallEnd();
  void incomingNotification(QString friendId);
  void onRejectCall(QString friendId);
  void onStopNotification();



  void registerContentDialog(ContentDialog &contentDialog) const;
  void friendRequestedTo(const ToxId &friendAddress,  const QString &nick, const QString &message);

private:
  // QMainWindow overrides
  bool eventFilter(QObject *obj, QEvent *event) final override;
  bool event(QEvent *e) final override;
  void closeEvent(QCloseEvent *event) final override;
  void changeEvent(QEvent *event) final override;
  void resizeEvent(QResizeEvent *event) final override;
  void moveEvent(QMoveEvent *event) final override;


  void setActiveToolMenuButton(ActiveToolMenuButton newActiveButton);
  void hideMainForms(GenericChatroomWidget *chatroomWidget);
  GroupWidget *createGroup(QString groupnumber, const GroupId &groupId, const QString& name);
  void removeFriend(Friend *f, bool fake = false);
  void removeGroup(Group *g, bool fake = false);
  void saveWindowGeometry();
  void saveSplitterGeometry();
  void cycleContacts(bool forward);

  void retranslateUi();
  void focusChatInput();
  void openDialog(GenericChatroomWidget *widget, bool newWindow);
  void playNotificationSound(IAudioSink::Sound sound, bool loop = false);
  void cleanupNotificationSound();
void connectToCore(Core&core);
private:

  std::unique_ptr<QSystemTrayIcon> icon;
  QMenu *trayMenu;
  QAction *statusOnline;
  QAction *statusAway;
  QAction *statusBusy;
  QAction *actionLogout;
  QAction *actionQuit;
  QAction *actionShow;
  void setStatusOnline();
  void setStatusAway();
  void setStatusBusy();

  Ui::IMMainWindow *ui;
  QSplitter *centralLayout;
  QPoint dragPosition;



  std::unique_ptr<ChatWidget > chatWidget;
  std::unique_ptr<ContactWidget> contactWidget;
  std::unique_ptr<SettingsWidget> settingsWidget;



  Core *core;
  Profile *profile;
  ProfileInfo *profileInfo;
  ProfileForm *profileForm;

  std::unique_ptr<UpdateCheck> updateCheck; // ownership should be moved outside
                                            // Widget once non-singleton
//  FilesForm *filesForm;

//  GenericChatroomWidget *activeChatroomWidget;
//  FriendListWidget *contactListWidget;
//  MaskablePixmapWidget *profilePicture;
  bool notify(QObject *receiver, QEvent *event);
  bool autoAwayActive = false;
  QTimer *timer;
  bool eventFlag;
  bool eventIcon;
  bool wasMaximized = false;

  int icon_size;

  IAudioControl &audio;
  std::unique_ptr<IAudioSink> audioNotification = nullptr;
  Settings &settings;

//  QMap<ToxPk, FriendWidget *> friendWidgets;
//  FriendListWidget* friendListWidget;
  // Shared pointer because qmap copies stuff all over the place
//  QMap<ToxPk, std::shared_ptr<FriendMessageDispatcher>>
//      friendMessageDispatchers;
  // Stop gap method of linking our friend messages back to a group id.
  // Eventual goal is to have a notification manager that works on
  // Messages hooked up to message dispatchers but we aren't there
  // yet
//  QMap<ToxPk, QMetaObject::Connection> friendAlertConnections;
//  QMap<ToxPk, std::shared_ptr<ChatHistory>> friendChatLogs;
//  QMap<ToxPk, std::shared_ptr<FriendChatroom>> friendChatrooms;
//  QMap<ToxPk, ChatForm *> chatForms;

//  QMap<GroupId, GroupWidget *> groupWidgets;
//  QMap<GroupId, std::shared_ptr<GroupMessageDispatcher>>
//      groupMessageDispatchers;

  // Stop gap method of linking our group messages back to a group id.
  // Eventual goal is to have a notification manager that works on
  // Messages hooked up to message dispatchers but we aren't there
  // yet
//  QMap<GroupId, QMetaObject::Connection> groupAlertConnections;
//  QMap<GroupId, std::shared_ptr<IChatLog>> groupChatLogs;
//  QMap<GroupId, std::shared_ptr<GroupChatroom>> groupChatrooms;
//  QMap<GroupId, QSharedPointer<GroupChatForm>> groupChatForms;



#if DESKTOP_NOTIFICATIONS
  DesktopNotify notifier;
#endif

#ifdef Q_OS_MAC
  QAction *fileMenu;
  QAction *editMenu;
  QAction *contactMenu;
  QMenu *changeStatusMenu;
  QAction *editProfileAction;
  QAction *logoutAction;
  QAction *addContactAction;
  QAction *nextConversationAction;
  QAction *previousConversationAction;
#endif

  std::shared_ptr<::base::DelayedCallTimer> delayCaller;
};

bool toxActivateEventHandler(const QByteArray &data);

#endif // WIDGET_H
