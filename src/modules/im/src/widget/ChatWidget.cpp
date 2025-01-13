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
// Created by gaojie on 24-5-7.
//

#include "ChatWidget.h"
#include <QMenu>
#include <QPainter>
#include <QSvgRenderer>
#include "ContactListWidget.h"
#include "MessageSessionListWidget.h"
#include "application.h"
#include "base/SvgUtils.h"
#include "base/utils.h"
#include "circlewidget.h"
#include "contentdialogmanager.h"
#include "contentlayout.h"
#include "lib/storage/settings/style.h"
#include "lib/storage/settings/translator.h"
#include "src/core/corefile.h"
#include "src/model/friendlist.h"
#include "src/model/group.h"
#include "src/model/groupinvite.h"
#include "src/model/grouplist.h"
#include "src/model/profile/profileinfo.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/widget/form/addfriendform.h"
#include "src/widget/form/groupinviteform.h"
#include "src/widget/form/profileform.h"
#include "ui_ChatWidget.h"
#include "widget.h"

#include "src/Bus.h"
#include "src/core/coreav.h"

namespace {

/**
 * @brief Dangerous way to find out if a path is writable.
 * @param filepath Path to file which should be deleted.
 * @return True, if file writeable, false otherwise.
 */
bool tryRemoveFile(const QString& filepath) {
    QFile tmp(filepath);
    bool writable = tmp.open(QIODevice::WriteOnly);
    tmp.remove();
    return writable;
}

void acceptFileTransfer(ToxFile& file, const QString& path) {
    QString filepath;
    int number = 0;

    QString suffix = QFileInfo(file.fileName).completeSuffix();
    QString base = QFileInfo(file.fileName).baseName();

    do {
        filepath =
                QString("%1/%2%3.%4")
                        .arg(path, base,
                             number > 0 ? QString("(%1)").arg(QString::number(number)) : QString(),
                             suffix);
        ++number;
    } while (QFileInfo(filepath).exists());

    file.setFilePath(filepath);
    // Do not automatically accept the file-transfer if the path is not writable.
    // The user can still accept it manually.
    if (tryRemoveFile(filepath)) {
        CoreFile* coreFile = CoreFile::getInstance();
        coreFile->acceptFileRecvRequest(file.receiver, file.fileId, filepath);
    } else {
        qWarning() << "Cannot write to " << filepath;
    }
}
}  // namespace

ChatWidget::ChatWidget(QWidget* parent)
        : MainLayout(parent)
        , ui(new Ui::ChatWidget)
        , unreadGroupInvites{0}
        , profileInfo{nullptr}
        , profileForm{nullptr} {
    ui->setupUi(this);
    layout()->setMargin(0);
    layout()->setSpacing(0);

    // 右侧容器
    contentWidget = std::make_unique<QWidget>(this);
    contentWidget->setObjectName("ChatContentWidget");
    contentLayout = new ContentLayout(contentWidget.get());
    contentWidget->setLayout(contentLayout);

    // 左侧
    sessionListWidget = std::make_unique<MessageSessionListWidget>(this, contentLayout, false);
    sessionListWidget->setGeometry(0, 0, 400, 400);
    sessionListWidget->layout()->setAlignment(Qt::AlignTop | Qt::AlignVCenter);

    ui->scrollAreaWidgetContents->setGeometry(0, 0, 200, 500);
    ui->scrollAreaWidgetContents->layout()->setAlignment(Qt::AlignTop | Qt::AlignVCenter);
    ui->scrollAreaWidgetContents->layout()->addWidget((QWidget*)sessionListWidget.get());

    ui->mainSplitter->addWidget(contentWidget.get());
    ui->mainSplitter->setSizes(QList<int>() << 200 << 500);
    ui->mainSplitter->setStretchFactor(1, 0);
    ui->mainSplitter->setChildrenCollapsible(false);

    //    auto s = Nexus::getProfile()->getSettings();

    setupStatus();
    setupSearch();
    init();
    retranslateUi();
}

ChatWidget::~ChatWidget() {
    deinit();
    delete ui;
}

void ChatWidget::init() {
    connect(ui->nameLabel, &CroppingLabel::clicked, this, &ChatWidget::on_nameClicked);

    auto widget = Widget::getInstance();

    connect(widget, &Widget::forwardMessage, this, &ChatWidget::doForwardMessage);
    connect(widget, &Widget::toSendMessage, this, &ChatWidget::doSendMessage);
    connect(widget, &Widget::friendAdded, this, &ChatWidget::onFriendAdded);
    connect(widget, &Widget::friendRemoved, this, &ChatWidget::onFriendRemoved);
    connect(widget, &Widget::groupAdded, this, &ChatWidget::onGroupAdded);
    connect(widget, &Widget::groupRemoved, this, &ChatWidget::onGroupRemoved);

    ok::Bus* bus = ok::Application::Instance()->bus();
    connect(bus, &ok::Bus::coreChanged, this, [&](Core* core) { connectToCore(core); });

    connect(bus, &ok::Bus::coreAvChanged, this, &ChatWidget::connectToCoreAv);

    connect(bus, &ok::Bus::coreFileChanged, this, &ChatWidget::connectToCoreFile);

    connect(bus, &ok::Bus::profileChanged, this, &ChatWidget::onProfileChanged);

    settings::Translator::registerHandler([this] { retranslateUi(); }, this);
}

void ChatWidget::deinit() {
    settings::Translator::unregister(this);

    disconnect(ui->nameLabel, &CroppingLabel::clicked, this, &ChatWidget::on_nameClicked);

    auto widget = Widget::getInstance();

    disconnect(widget, &Widget::toSendMessage, this, &ChatWidget::doSendMessage);
    disconnect(widget, &Widget::friendAdded, this, &ChatWidget::onFriendAdded);
    disconnect(widget, &Widget::friendRemoved, this, &ChatWidget::onFriendRemoved);
    disconnect(widget, &Widget::groupAdded, this, &ChatWidget::onGroupAdded);
    disconnect(widget, &Widget::groupRemoved, this, &ChatWidget::onGroupRemoved);

    delete profileForm;
    delete profileInfo;
}

void ChatWidget::connectToCore(Core* core) {
    qDebug() << __func__ << "core:" << core;

    connect(core, &Core::connecting, this, &ChatWidget::onConnecting );
    connect(core, &Core::connected, this, &ChatWidget::onConnected );
    connect(core, &Core::disconnected, this, &ChatWidget::onDisconnected );


    connect(core, &Core::usernameSet, this, &ChatWidget::onNicknameSet);
    connect(core, &Core::statusSet, this, &ChatWidget::onStatusSet);
    connect(core, &Core::statusMessageSet, this, &ChatWidget::onStatusMessageSet);
    connect(core, &Core::messageSessionReceived, this, &ChatWidget::onMessageSessionReceived);
    connect(core, &Core::friendNicknameChanged, this, &ChatWidget::onFriendNickChanged);
    connect(core, &Core::friendAvatarChanged, this, &ChatWidget::onFriendAvatarChanged);
    connect(core, &Core::friendMessageReceived, this, &ChatWidget::onFriendMessageReceived);
    connect(core, &Core::friendStatusChanged, this, &ChatWidget::onFriendStatusChanged);
    connect(core, &Core::friendStatusMessageChanged, this, &ChatWidget::onFriendStatusMessageChanged);
    connect(core, &Core::friendTypingChanged, this, &ChatWidget::onFriendTypingChanged);
    connect(core, &Core::receiptRecieved, this, &ChatWidget::onReceiptReceived);
    connect(core, &Core::groupMessageReceived, this, &ChatWidget::onGroupMessageReceived);
    connect(core, &Core::groupPeerlistChanged, this, &ChatWidget::onGroupPeerListChanged);
    connect(core, &Core::groupPeerSizeChanged, this, &ChatWidget::onGroupPeerSizeChanged);
    connect(core, &Core::groupPeerNameChanged, this, &ChatWidget::onGroupPeerNameChanged);
    connect(core, &Core::groupPeerStatusChanged, this, &ChatWidget::onGroupPeerStatusChanged);

}

void ChatWidget::connectToCoreFile(CoreFile* coreFile) {
    connect(coreFile, &CoreFile::fileSendStarted, this, &ChatWidget::dispatchFile);
    connect(coreFile, &CoreFile::fileReceiveRequested, this, &ChatWidget::dispatchFile);
    connect(coreFile, &CoreFile::fileTransferAccepted, this, &ChatWidget::dispatchFile);
    connect(coreFile, &CoreFile::fileTransferCancelled, this, &ChatWidget::dispatchFile);
    connect(coreFile, &CoreFile::fileTransferFinished, this, &ChatWidget::dispatchFile);
    connect(coreFile, &CoreFile::fileTransferPaused, this, &ChatWidget::dispatchFile);
    connect(coreFile, &CoreFile::fileTransferInfo, this, &ChatWidget::dispatchFile);
    connect(coreFile, &CoreFile::fileTransferNoExisting, this, &ChatWidget::cancelFile);

    connect(coreFile, &CoreFile::fileTransferRemotePausedUnpaused, this,
            &ChatWidget::dispatchFileWithBool);
    connect(coreFile, &CoreFile::fileTransferBrokenUnbroken, this,
            &ChatWidget::dispatchFileWithBool);
    connect(coreFile, &CoreFile::fileSendFailed, this, &ChatWidget::dispatchFileSendFailed);
}

void ChatWidget::connectToCoreAv(CoreAV* coreAv) {
    connect(coreAv, &CoreAV::avInvite, this, &ChatWidget::onAvInvite);
    connect(coreAv, &CoreAV::avStart, this, &ChatWidget::onAvStart);
    connect(coreAv, &CoreAV::avPeerConnectionState, this, &ChatWidget::onAvPeerConnectionState);
    connect(coreAv, &CoreAV::avEnd, this, &ChatWidget::onAvEnd);
}

void ChatWidget::onMessageSessionReceived(const ContactId& contactId, const QString& sid) {
    sessionListWidget->createMessageSession(
            contactId, sid, contactId.isGroup() ? ChatType::GroupChat : ChatType::Chat);
}

void ChatWidget::onFriendMessageReceived(const FriendId friendId,
                                         const FriendMessage message,
                                         bool isAction) {
    sessionListWidget->setRecvFriendMessage(friendId, message, isAction);
}

void ChatWidget::onReceiptReceived(const FriendId& friendId, MsgId receipt) {
    sessionListWidget->setFriendMessageReceipt(friendId, receipt);
}

void ChatWidget::onFriendStatusChanged(const FriendId& friendPk, Status::Status status) {
    Friend* f = Nexus::getCore()->getFriendList().findFriend(friendPk);
    if (!f) {
        qWarning() << "Unable to find friend" << friendPk.toString();
        return;
    }
    f->setStatus(status);
}

void ChatWidget::onFriendStatusMessageChanged(const FriendId& friendPk, const QString& message) {
    sessionListWidget->setFriendStatusMsg(friendPk, message);
}

void ChatWidget::onFriendTypingChanged(const FriendId& friendId, bool isTyping) {
    sessionListWidget->setFriendTyping(friendId, isTyping);
}

void ChatWidget::onGroupAdded(const Group* g) {
    sessionListWidget->addGroup(g);
}

void ChatWidget::onGroupRemoved(const Group* g) {
    sessionListWidget->removeGroup(g);
}

void ChatWidget::showEvent(QShowEvent* e) {}

void ChatWidget::onNicknameSet(const QString& nickname) {
    qDebug() << __func__ << nickname;

    ui->nameLabel->setText(nickname);
    ui->nameLabel->setToolTip(Qt::convertFromPlainText(nickname, Qt::WhiteSpaceNormal));

    // 修改消息列表自己的昵称
    auto core = Nexus::getInstance()->getProfile()->getCore();
    sessionListWidget->setFriendName(core->getSelfId(), nickname);
}

void ChatWidget::onStatusSet(Status::Status status) {
    int icon_size = 15;
    ui->statusButton->setProperty("status", static_cast<int>(status));
    ui->statusButton->setIcon(ok::base::SvgUtils::prepareIcon(getIconPath(status), icon_size, icon_size));
    updateIcons();
}

void ChatWidget::updateIcons() {
    QIcon ico;
    bool eventIcon = true;

    const QString assetSuffix = Status::getAssetSuffix(static_cast<Status::Status>(
                                        ui->statusButton->property("status").toInt())) +
                                (eventIcon ? "_event" : "");

    QString color = Nexus::getProfile()->getSettings()->getLightTrayIcon() ? "light" : "dark";
    QString path = ":/img/taskbar/" + color + "/taskbar_" + assetSuffix + ".svg";
    QSvgRenderer renderer(path);

    // Prepare a QImage with desired characteritisc
    QImage image = QImage(250, 250, QImage::Format_ARGB32);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    renderer.render(&painter);
    ico = QIcon(QPixmap::fromImage(image));
    setWindowIcon(ico);
}

void ChatWidget::onStatusMessageSet(const QString& statusMessage) {
    ui->statusLabel->setText(statusMessage);
}

void ChatWidget::onFriendAdded(const Friend* f) {
    sessionListWidget->setFriend(f);
}

void ChatWidget::onFriendRemoved(const Friend* f) {
    sessionListWidget->removeFriend(f);
}

void ChatWidget::doSendMessage(const QString& to, bool isGroup) {
    sessionListWidget->toSendMessage(FriendId(to), isGroup);
}

void ChatWidget::doForwardMessage(const ContactId& cid, const MsgId& msgId) {
    sessionListWidget->toForwardMessage(cid, msgId);
}

// void ChatWidget::onGroupInviteReceived(const GroupInvite &inviteInfo) {

//  auto confType = inviteInfo.getType();
//  if (confType == ConferenceType::TEXT || confType == ConferenceType::AV) {
//    if (false
//        // settings.getAutoGroupInvite(f->getPublicKey())
//    ) {
//      onGroupInviteAccepted(inviteInfo);
//    } else {
//      if (!groupInviteForm->addGroupInvite(inviteInfo)) {
//        return;
//      }

//      ++unreadGroupInvites;
//      groupInvitesUpdate();
//      Widget::getInstance()->newMessageAlert(window(), isActiveWindow(), true, true);

// #if DESKTOP_NOTIFICATIONS
//       if (settings.getNotifyHide()) {
//         notifier.notifyMessageSimple(DesktopNotify::MessageType::GROUP_INVITE);
//       } else {
//         notifier.notifyMessagePixmap(f->getDisplayedName() + tr(" invites you to join a group."),
//         {}, Nexus::getProfile()->loadAvatar(f->getPublicKey()));
//       }
// #endif
//     }
//   } else {
//     qWarning() << "onGroupInviteReceived: Unknown ConferenceType:" << (int)confType;
//     return;
//   }
// }

void ChatWidget::onGroupInviteAccepted(const GroupInvite& inviteInfo) {
    const QString groupId = Nexus::getCore()->joinGroupchat(inviteInfo);
    qDebug() << "onGroupInviteAccepted groupId=>" << groupId;

    if (groupId == std::numeric_limits<uint32_t>::max()) {
        qWarning() << "onGroupInviteAccepted: Unable to accept group invite";
        return;
    }
}

void ChatWidget::onGroupMessageReceived(GroupId groupId, GroupMessage msg) {
    qDebug() << __func__ << msg.toString();
    sessionListWidget->setRecvGroupMessage(groupId, msg);
}

void ChatWidget::onGroupPeerListChanged(QString groupnumber) {
    //  const GroupId &groupId = GroupList::id2Key(groupnumber);
    //  Group *g = GroupList::findGroup(groupId);
    //  assert(g);
    //  g->regeneratePeerList();
}

void ChatWidget::onGroupPeerSizeChanged(QString groupnumber, const uint size) {
    const GroupId& groupId = GroupId(groupnumber);
    Group* g = GroupList::findGroup(groupId);
    if (!g) {
        qWarning() << "Can not find the group named:" << groupnumber;
        return;
    }
    g->setPeerCount(size);
}

void ChatWidget::onGroupPeerNameChanged(QString groupnumber, const FriendId& peerPk,
                                        const QString& newName) {
    const GroupId& groupId = GroupId(groupnumber);
    Group* g = GroupList::findGroup(groupId);
    if (!g) {
        qWarning() << "Can not find the group named:" << groupnumber;
        return;
    }
    //  const QString &setName = FriendList::decideNickname(peerPk, newName);
    //  g->updateUsername(peerPk, newName);
}

void ChatWidget::onGroupPeerStatusChanged(const QString& groupnumber, const GroupOccupant& go) {
    const GroupId& groupId = GroupId(groupnumber);
    Group* g = GroupList::findGroup(groupId);
    if (!g) {
        qWarning() << "Can not find group named:" << groupId.username;
        return;
    }

    g->addPeer(go);
}

void ChatWidget::onGroupTitleChanged(QString groupnumber, const QString& author,
                                     const QString& title) {
    qDebug() << __func__ << "group" << groupnumber << title;
    const GroupId& groupId = GroupId(groupnumber);
    Group* g = GroupList::findGroup(groupId);
    if (!g) {
        qWarning() << "Can not find group" << groupnumber;
        return;
    }

    //  contactListWidget->setGroupTitle(groupId, author, title);

    //  FilterCriteria filter = getFilterCriteria();
    //  widget->searchName(ui->searchContact->text(), filterGroups(filter));
}

void ChatWidget::groupInvitesUpdate() {
    if (unreadGroupInvites == 0) {
        delete groupInvitesButton;
        groupInvitesButton = nullptr;
    } else if (!groupInvitesButton) {
        groupInvitesButton = new QPushButton(this);
        groupInvitesButton->setObjectName("green");
        //    ui->statusLayout->insertWidget(2, groupInvitesButton);

        connect(groupInvitesButton, &QPushButton::released, this, &ChatWidget::onGroupClicked);
    }

    if (groupInvitesButton) {
        groupInvitesButton->setText(tr("%n New Group Invite(s)", "", unreadGroupInvites));
    }
}

void ChatWidget::groupInvitesClear() {
    unreadGroupInvites = 0;
    groupInvitesUpdate();
}

void ChatWidget::showProfile() {
    auto profile = Nexus::getProfile();
    if (!profileForm) {
        profileInfo = new ProfileInfo(Nexus::getCore(), profile);
        profileForm = new ProfileForm(profileInfo);
    }

    if (!profileForm->isShown()) {
        profileForm->showTo(getContentLayout());
    }
}

void ChatWidget::clearAllReceipts() {
    sessionListWidget->clearAllReceipts();
}

void ChatWidget::on_nameClicked() {
    qDebug() << __func__;
    showProfile();
}

void ChatWidget::onProfileChanged(Profile* profile) {
    connect(profile, &Profile::nickChanged, this, &ChatWidget::onNicknameSet);

    QList<MessageSession> mss;
    profile->getHistory()->getMessageSessions(mss);

    for(auto &p: mss){
        //TODO ChatType::Chat
        sessionListWidget->createMessageSession(ContactId(p.peer_jid), p.session_id, ChatType::Chat);
    }
}

void ChatWidget::onConnecting()
{
    //TODO IM模块，"连接中"
    qDebug() << __func__;
}

void ChatWidget::onDisconnected(int err)
{
    //TODO IM模块，"已断开(err)"
    qDebug() << __func__;
}

void ChatWidget::onConnected()
{
    //TODO IM模块，"在线"
    qDebug() << __func__;
}

void ChatWidget::onGroupClicked() {

}

void ChatWidget::reloadTheme() {
    setStyleSheet(lib::settings::Style::getStylesheet("window/chat.css"));
    QString statusPanelStyle = lib::settings::Style::getStylesheet("window/statusPanel.css");
    ui->statusHead->setStyleSheet(statusPanelStyle);
    ui->friendList->setStyleSheet(lib::settings::Style::getStylesheet("friendList/friendList.css"));
    ui->statusButton->setStyleSheet(
            lib::settings::Style::getStylesheet("statusButton/statusButton.css"));
    sessionListWidget->reDraw();

    if (contentLayout != nullptr) {
        contentLayout->reloadTheme();
    }

    //  for (IMFriend *f : FriendList::getAllFriends()) {
    //    contactListWidget->getFriend(f->getPublicKey())->reloadTheme();
    //  }

    sessionListWidget->reloadTheme();

    ui->friendList->setAutoFillBackground(false);
    ui->friendList->viewport()->setAutoFillBackground(false);
}

void ChatWidget::setupSearch() {
    ui->searchContact->setPlaceholderText(tr("Search Contacts"));
    connect(ui->searchContact, &QLineEdit::textChanged, this, &ChatWidget::searchContacts);
}

void ChatWidget::searchContacts() {
    QString text = ui->searchContact->text();
    sessionListWidget->search(text);
    sessionListWidget->reDraw();
}

void ChatWidget::retranslateUi() {
    ui->searchContact->setPlaceholderText(tr("Search Contacts"));
    ui->retranslateUi(this);

    statusOnline->setText(tr("Online", "Button to set your status to 'Online'"));
    statusAway->setText(tr("Away", "Button to set your status to 'Away'"));
    statusBusy->setText(tr("Busy", "Button to set your status to 'Busy'"));
}

void ChatWidget::setupStatus() {
    int icon_size = 15;

    // Preparing icons and set their size
    statusOnline = new QAction(this);
    statusOnline->setIcon(ok::base::SvgUtils::prepareIcon(
            Status::getIconPath(Status::Status::Online), icon_size, icon_size));
    connect(statusOnline, &QAction::triggered, this, &ChatWidget::setStatusOnline);

    statusAway = new QAction(this);
    statusAway->setIcon(ok::base::SvgUtils::prepareIcon(Status::getIconPath(Status::Status::Away),
                                                        icon_size, icon_size));
    connect(statusAway, &QAction::triggered, this, &ChatWidget::setStatusAway);

    statusBusy = new QAction(this);
    statusBusy->setIcon(ok::base::SvgUtils::prepareIcon(Status::getIconPath(Status::Status::Busy),
                                                        icon_size, icon_size));
    connect(statusBusy, &QAction::triggered, this, &ChatWidget::setStatusBusy);

    QMenu* statusButtonMenu = new QMenu(ui->statusButton);
    statusButtonMenu->addAction(statusOnline);
    statusButtonMenu->addAction(statusAway);
    statusButtonMenu->addAction(statusBusy);
    ui->statusButton->setMenu(statusButtonMenu);

    statusOnline->setText(tr("Online", "Button to set your status to 'Online'"));
    statusAway->setText(tr("Away", "Button to set your status to 'Away'"));
    statusBusy->setText(tr("Busy", "Button to set your status to 'Busy'"));
}

void ChatWidget::cancelFile(const QString& friendId, const QString& fileId) {
    qDebug() << __func__ << "file:" << fileId;
    auto frndId = FriendId{friendId};
    sessionListWidget->setFriendFileCancelled(frndId, fileId);
}

void ChatWidget::dispatchFile(ToxFile file) {
    qDebug() << __func__ << "file:" << file.toString();

    const auto& cId = ContactId(file.getFriendId());

    if (file.status == FileStatus::INITIALIZING && file.direction == FileDirection::RECEIVING) {
        auto settings = Nexus::getProfile()->getSettings();
        //    QString autoAcceptDir = settings.getAutoAcceptDir(cId);
        //    if (autoAcceptDir.isEmpty() &&
        //    lib::settings::OkNexus::getSettings()->getAutoSaveEnabled()) {
        auto autoAcceptDir = settings->getGlobalAutoAcceptDir();
        //    }

        //    auto maxAutoAcceptSize = settings.getMaxAutoAcceptSize();
        //    bool autoAcceptSizeCheckPassed = maxAutoAcceptSize == 0 || maxAutoAcceptSize >=
        //    file.fileSize;

        //    if (!autoAcceptDir.isEmpty() && autoAcceptSizeCheckPassed) {
        acceptFileTransfer(file, autoAcceptDir);
    }
    sessionListWidget->setFriendFileReceived(cId, file);
}

void ChatWidget::dispatchFileWithBool(ToxFile file, bool) {
    dispatchFile(file);
}

void ChatWidget::dispatchFileSendFailed(
        QString friendId, const QString& fileName) {  //  const auto &friendPk = ToxPk(receiver);

    // TODO
    //   chatForm.value()->addSystemInfoMessage(
    //       tr("Failed to send file \"%1\"").arg(fileName), ChatMessage::ERROR,
    //       QDateTime::currentDateTime());
}

void ChatWidget::setStatusOnline() {
    //  if (!ui->statusButton->isEnabled()) {
    //    return;
    //  }
    Nexus::getCore()->setStatus(Status::Status::Online);
}

void ChatWidget::setStatusAway() {
    //  if (!ui->statusButton->isEnabled()) {
    //    return;
    //  }

    Nexus::getCore()->setStatus(Status::Status::Away);
}

void ChatWidget::setStatusBusy() {
    //  if (!ui->statusButton->isEnabled()) {
    //    return;
    //  }

    Nexus::getCore()->setStatus(Status::Status::Busy);
}

void ChatWidget::onAvInvite(ToxPeer peerId, bool video) {
    qDebug() << __func__ << "friendId" << peerId << video;
    sessionListWidget->setFriendAvInvite(peerId, video);

    //  auto testedFlag = video ?
    //              Settings::AutoAcceptCall::Video : Settings::AutoAcceptCall::Audio;

    //  // AutoAcceptCall is set for this friend
    //  if (Nexus::getProfile()->getSettings()
    //          .getAutoAcceptCall(*f)
    //          .testFlag(testedFlag)) {

    //    CoreAV *coreav = CoreAV::getInstance();
    //    QMetaObject::invokeMethod(coreav, "answerCall", Qt::QueuedConnection,
    //                              Q_ARG(ToxPeer, peerId), Q_ARG(bool, video));

    //    onAvStart(friendId, video);
    //  } else {
    //    headWidget->createCallConfirm(peerId, video);
    //    headWidget->showCallConfirm();
    //    lastCallIsVideo = video;
    //    emit incomingNotification(fId);
    //  }
}

void ChatWidget::onAvStart(const FriendId& friendId, bool video) {
    qDebug() << __func__ << "friendId:" << friendId;
    sessionListWidget->setFriendAvStart(friendId, video);
}

void ChatWidget::onAvPeerConnectionState(const FriendId& friendId,
                                         lib::ortc::PeerConnectionState state) {
    qDebug() << __func__ << "friendId:" << friendId << "state:" << static_cast<int>(state);
    sessionListWidget->setFriendAvPeerConnectedState(friendId, state);
}

void ChatWidget::onAvEnd(const FriendId& friendId, bool error) {
    qDebug() << __func__ << "friendId:" << friendId << "error:" << error;
    sessionListWidget->setFriendAvEnd(friendId, error);
}

void ChatWidget::onFriendNickChanged(const FriendId& friendPk, const QString& nickname) {
    sessionListWidget->setFriendName(friendPk, nickname);
}

void ChatWidget::onFriendAvatarChanged(const FriendId& friendPk, const QByteArray& avatar) {
    sessionListWidget->setFriendAvatar(friendPk, avatar);
}
