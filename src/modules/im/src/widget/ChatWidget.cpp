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
#include <src/model/profile/profileinfo.h>
#include <src/widget/form/profileform.h>
#include <QMenu>
#include <QPainter>
#include <QSvgRenderer>
#include "MessageSessionListWidget.h"
#include "application.h"
#include "base/OkSettings.h"
#include "base/SvgUtils.h"
#include "base/utils.h"
#include "circlewidget.h"
#include "contentdialogmanager.h"
#include "contentlayout.h"
#include "friendlistwidget.h"
#include "lib/settings/translator.h"
#include "src/core/corefile.h"
#include "src/friendlist.h"
#include "src/lib/settings/style.h"
#include "src/model/group.h"
#include "src/model/groupinvite.h"
#include "src/modules/im/src/grouplist.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/widget/form/addfriendform.h"
#include "src/widget/form/groupinviteform.h"
#include "ui_ChatWidget.h"
#include "widget.h"

#include <src/core/coreav.h>
#include "src/Bus.h"

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
        CoreFile* coreFile = Core::getInstance()->getCoreFile();
        coreFile->acceptFileRecvRequest(file.receiver, file.fileId, filepath);
    } else {
        qWarning() << "Cannot write to " << filepath;
    }
}
}  // namespace

ChatWidget::ChatWidget(QWidget* parent)
        : MainLayout(parent)
        ,  //
        ui(new Ui::ChatWidget)
        ,  //
        unreadGroupInvites{0}
        , core{nullptr}
        , coreFile{nullptr}
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

    const Settings& s = Settings::getInstance();

    reloadTheme();
    setupStatus();
    setupSearch();
    init();

    retranslateUi();

    // todo: delete from ui
    ui->searchContactFilterBox->setVisible(false);
}

ChatWidget::~ChatWidget() {
    deinit();
    delete ui;
}

void ChatWidget::init() {
    connect(ui->nameLabel, &CroppingLabel::clicked, this, &ChatWidget::on_nameClicked);

    auto widget = Widget::getInstance();

    connect(widget, &Widget::toSendMessage, this, &ChatWidget::doSendMessage);
    connect(widget, &Widget::friendAdded, this, &ChatWidget::onFriendAdded);
    connect(widget, &Widget::friendRemoved, this, &ChatWidget::onFriendRemoved);
    connect(widget, &Widget::groupAdded, this, &ChatWidget::onGroupAdded);
    connect(widget, &Widget::groupRemoved, this, &ChatWidget::onGroupRemoved);

    connect(ok::Application::Instance()->bus(), &ok::Bus::coreChanged, this,
            [&](Core* core) { connectToCore(core); });

    connect(ok::Application::Instance()->bus(), &ok::Bus::coreFileChanged, this,
            [&](CoreFile* coreFile) { connectToCoreFile(coreFile); });

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

void ChatWidget::connectToCore(Core* core_) {
    qDebug() << __func__ << "core:" << core_;
    core = core_;  // TODO: 待优化
    connect(Nexus::getProfile(), &Profile::nickChanged, this, &ChatWidget::onUsernameSet);

    connect(core_, &Core::statusSet, this, &ChatWidget::onStatusSet);
    connect(core_, &Core::statusMessageSet, this, &ChatWidget::onStatusMessageSet);
    connect(core_, &Core::messageSessionReceived, this, &ChatWidget::onMessageSessionReceived);
    connect(core_, &Core::friendUsernameChanged, this, &ChatWidget::onFriendNickChanged);
    connect(core_, &Core::friendAvatarChanged, this, &ChatWidget::onFriendAvatarChanged);
    connect(core_, &Core::friendMessageReceived, this, &ChatWidget::onFriendMessageReceived);
    connect(core_, &Core::friendStatusChanged, this, &ChatWidget::onFriendStatusChanged);
    connect(core_, &Core::friendStatusMessageChanged, this,
            &ChatWidget::onFriendStatusMessageChanged);
    connect(core_, &Core::friendTypingChanged, this, &ChatWidget::onFriendTypingChanged);
    connect(core_, &Core::receiptRecieved, this, &ChatWidget::onReceiptReceived);
    connect(core_, &Core::groupMessageReceived, this, &ChatWidget::onGroupMessageReceived);
    connect(core_, &Core::groupPeerlistChanged, this, &ChatWidget::onGroupPeerListChanged);
    connect(core_, &Core::groupPeerSizeChanged, this, &ChatWidget::onGroupPeerSizeChanged);
    connect(core_, &Core::groupPeerNameChanged, this, &ChatWidget::onGroupPeerNameChanged);
    connect(core_, &Core::groupPeerStatusChanged, this, &ChatWidget::onGroupPeerStatusChanged);

    //    connect(core_, &Core::groupPeerAudioPlaying, this,
    //            &ChatWidget::onGroupPeerAudioPlaying);
    //    connect(core_, &Core::emptyGroupCreated, this,
    //           &ChatWidget::onEmptyGroupCreated);

    //    connect(&core_, &Core::groupSentFailed, this,
    //            &ChatWidget::onGroupSendFailed);
}

void ChatWidget::connectToCoreFile(CoreFile* coreFile) {
    connect(coreFile, &CoreFile::fileSendStarted, this, &ChatWidget::dispatchFile);
    connect(coreFile, &CoreFile::fileReceiveRequested, this, &ChatWidget::dispatchFile);
    connect(coreFile, &CoreFile::fileTransferAccepted, this, &ChatWidget::dispatchFile);
    connect(coreFile, &CoreFile::fileTransferCancelled, this, &ChatWidget::dispatchFile);
    connect(coreFile, &CoreFile::fileTransferNoExisting, this, &ChatWidget::cancelFile);
    connect(coreFile, &CoreFile::fileTransferFinished, this, &ChatWidget::dispatchFile);
    connect(coreFile, &CoreFile::fileTransferPaused, this, &ChatWidget::dispatchFile);
    connect(coreFile, &CoreFile::fileTransferInfo, this, &ChatWidget::dispatchFile);
    connect(coreFile, &CoreFile::fileTransferRemotePausedUnpaused, this,
            &ChatWidget::dispatchFileWithBool);
    connect(coreFile, &CoreFile::fileTransferBrokenUnbroken, this,
            &ChatWidget::dispatchFileWithBool);
    connect(coreFile, &CoreFile::fileSendFailed, this, &ChatWidget::dispatchFileSendFailed);
}

void ChatWidget::onCoreChanged(Core& core_) {
    core = &core_;
    connectToCore(core);
    connectToCoreFile(core->getCoreFile());

    auto username = core->getUsername();
    qDebug() << "username" << username;
    ui->nameLabel->setText(username);

    const CoreAV* av = CoreAV::getInstance();
    connect(av, &CoreAV::avInvite, this, &ChatWidget::onAvInvite);
    connect(av, &CoreAV::avStart, this, &ChatWidget::onAvStart);
    connect(av, &CoreAV::avEnd, this, &ChatWidget::onAvEnd);
}

void ChatWidget::onMessageSessionReceived(const ContactId& contactId, const QString& sid) {
    qDebug() << __func__ << "contactId:" << contactId.toString() << "sid:" << sid;
    sessionListWidget->createMessageSession(
            contactId, sid, contactId.isGroup() ? ChatType::GroupChat : ChatType::Chat);
}

void ChatWidget::onFriendMessageReceived(const FriendId& friendId, const FriendMessage& message,
                                         bool isAction) {
    qDebug() << __func__ << "content:" << message.content << "from" << message.from;
    sessionListWidget->setRecvFriendMessage(friendId, message, isAction);
}

void ChatWidget::onReceiptReceived(const FriendId& friendId, MsgId receipt) {
    qDebug() << __func__ << "receiver:" << friendId.toString() << receipt;
    sessionListWidget->setFriendMessageReceipt(friendId, receipt);
}

void ChatWidget::onFriendStatusChanged(const FriendId& friendPk, Status::Status status) {
    qDebug() << __func__ << friendPk.toString() << "status:" << (int)status;
    //  const auto &friendPk = FriendList::id2Key(friendPk);
    //    contactListWidget->setFriendStatus(friendPk, status);
    Friend* f = FriendList::findFriend(friendPk);
    if (!f) {
        qWarning() << "Unable to find friend" << friendPk.toString();
        return;
    }
    f->setStatus(status);

    //  bool isActualChange = f->getStatus() != status;
    //
    //  FriendWidget *widget = // friendWidgets[f->getPublicKey()];
    //  contactListWidget->getFriend(f->getPublicKey());
    //  if (isActualChange) {
    //    if (!Status::isOnline(f->getStatus())) {
    //      contactListWidget->moveWidget(widget, Status::Status::Online);
    //    } else if (status == Status::Status::Offline) {
    //      contactListWidget->moveWidget(widget, Status::Status::Offline);
    //    }
    //  }
    //

    //  if (widget->isActive()) {
    //    setWindowTitle(widget->getSubject());
    //  }
    //
    //    ContentDialogManager::getInstance()->updateFriendStatus(friendPk);
}

void ChatWidget::onFriendStatusMessageChanged(const FriendId& friendPk, const QString& message) {
    sessionListWidget->setFriendStatusMsg(friendPk, message);

    //  IMFriend *f = FriendList::findFriend(friendPk);
    //  if (!f) {
    //    return;
    //  }
    //
    //  QString str = message;
    //  str.replace('\n', ' ').remove('\r').remove(QChar('\0'));
    //  f->setStatusMessage(str);
    //
    //  chatForms[friendPk]->setStatusMessage(str);
    //
    //  auto frd = contactListWidget->getFriend(friendPk);
    //  if(frd){
    //    frd->setStatusMsg(str);
    //  }
}

void ChatWidget::onFriendTypingChanged(const FriendId& friendId, bool isTyping) {
    sessionListWidget->setFriendTyping(friendId, isTyping);
}

void ChatWidget::onGroupAdded(const Group* g) { sessionListWidget->addGroup(g); }

void ChatWidget::onGroupRemoved(const Group* g) { sessionListWidget->removeGroup(g); }

void ChatWidget::showEvent(QShowEvent* e) {}

// void ChatWidget::onFriendDisplayedNameChanged(const ToxPk & friendPk, const
// QString &displayed) {
//    contactListWidget->setFriendName(friendPk, displayed);
// }

// void ChatWidget::onFriendAliasChanged(const ToxPk &receiver, const QString
// &alias) {
//   qDebug() << __func__ <<"receiver" << receiver.toString() << alias;
//   IMFriend *f = qobject_cast<IMFriend *>(sender());
//
//   // TODO(sudden6): don't update the contact list here, make it update itself
//   FriendWidget *friendWidget = contactListWidget->getFriend(receiver);
//   Status::Status status = f->getStatus();
//   contactListWidget->moveWidget(friendWidget, status);
//   FilterCriteria criteria = getFilterCriteria();
//   bool filter = status == Status::Status::Offline ? filterOffline(criteria)
//                                                   : filterOnline(criteria);
//   friendWidget->searchName(ui->searchContactText->text(), filter);
//
//   settings.setFriendAlias(receiver, alias);
//   settings.savePersonal();
//}

void ChatWidget::onUsernameSet(const QString& username) {
    qDebug() << __func__ << username;
    ui->nameLabel->setText(username);
    ui->nameLabel->setToolTip(Qt::convertFromPlainText(username, Qt::WhiteSpaceNormal));
    sessionListWidget->setFriendName(core->getSelfId(), username);
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

    QString color = Settings::getInstance().getLightTrayIcon() ? "light" : "dark";
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

void ChatWidget::onFriendAdded(const Friend* f) { sessionListWidget->addFriend(f); }

void ChatWidget::onFriendRemoved(const Friend* f) { sessionListWidget->removeFriend(f); }

void ChatWidget::doSendMessage(const QString& to, bool isGroup) {
    sessionListWidget->toSendMessage(FriendId(to), isGroup);
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
    const QString groupId = core->joinGroupchat(inviteInfo);
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
    //  widget->searchName(ui->searchContactText->text(), filterGroups(filter));
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
        auto core = Core::getInstance();
        profileInfo = new ProfileInfo(core, profile);
        profileForm = new ProfileForm(profileInfo);
    }

    if (!profileForm->isShown()) {
        profileForm->showTo(getContentLayout());
    }
}

void ChatWidget::clearAllReceipts() { sessionListWidget->clearAllReceipts(); }

void ChatWidget::on_nameClicked() {
    qDebug() << __func__;
    showProfile();
}

void ChatWidget::onGroupClicked() {
    auto& settings = Settings::getInstance();

    //    hideMainForms(nullptr);
    //  if (!groupInviteForm) {
    //    groupInviteForm = new GroupInviteForm;
    //    connect(groupInviteForm, &GroupInviteForm::groupCreate, core, &Core::createGroup);
    //  }
    //  groupInviteForm->show(contentLayout);
    //    setWindowTitle(fromDialogType(DialogType::GroupDialog));
    //    setActiveToolMenuButton(ActiveToolMenuButton::GroupButton);
}
void ChatWidget::reloadTheme() {
    setStyleSheet(Style::getStylesheet("window/chat.css"));
    QString statusPanelStyle = Style::getStylesheet("window/statusPanel.css");
    //  ui->tooliconsZone->setStyleSheet(
    //      Style::getStylesheet("tooliconsZone/tooliconsZone.css"));
    //  ui->statusPanel->setStyleSheet(statusPanelStyle);
    ui->statusHead->setStyleSheet(statusPanelStyle);
    ui->friendList->setStyleSheet(Style::getStylesheet("friendList/friendList.css"));
    ui->statusButton->setStyleSheet(Style::getStylesheet("statusButton/statusButton.css"));
    sessionListWidget->reDraw();

    //  profilePicture->setStyleSheet(Style::getStylesheet("window/profile.css"));

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
    filterMenu = new QMenu(this);
    filterGroup = new QActionGroup(this);
    filterDisplayGroup = new QActionGroup(this);

    filterDisplayName = new QAction(this);
    filterDisplayName->setCheckable(true);
    filterDisplayName->setChecked(true);
    filterDisplayGroup->addAction(filterDisplayName);
    filterMenu->addAction(filterDisplayName);
    filterDisplayActivity = new QAction(this);
    filterDisplayActivity->setCheckable(true);
    filterDisplayGroup->addAction(filterDisplayActivity);
    filterMenu->addAction(filterDisplayActivity);

    Settings::getInstance().getFriendSortingMode() == MessageSessionListWidget::SortingMode::Name
            ? filterDisplayName->setChecked(true)
            : filterDisplayActivity->setChecked(true);
    filterMenu->addSeparator();

    filterAllAction = new QAction(this);
    filterAllAction->setCheckable(true);
    filterAllAction->setChecked(true);
    filterGroup->addAction(filterAllAction);
    filterMenu->addAction(filterAllAction);
    filterOnlineAction = new QAction(this);
    filterOnlineAction->setCheckable(true);
    filterGroup->addAction(filterOnlineAction);
    filterMenu->addAction(filterOnlineAction);
    filterOfflineAction = new QAction(this);
    filterOfflineAction->setCheckable(true);
    filterGroup->addAction(filterOfflineAction);
    filterMenu->addAction(filterOfflineAction);
    filterFriendsAction = new QAction(this);
    filterFriendsAction->setCheckable(true);
    filterGroup->addAction(filterFriendsAction);
    filterMenu->addAction(filterFriendsAction);
    filterGroupsAction = new QAction(this);
    filterGroupsAction->setCheckable(true);
    filterGroup->addAction(filterGroupsAction);
    filterMenu->addAction(filterGroupsAction);

    filterDisplayName->setText(tr("By Name"));
    filterDisplayActivity->setText(tr("By Activity"));
    filterAllAction->setText(tr("All"));
    filterOnlineAction->setText(tr("Online"));
    filterOfflineAction->setText(tr("Offline"));
    filterFriendsAction->setText(tr("Friends"));
    filterGroupsAction->setText(tr("Groups"));

    ui->searchContactText->setPlaceholderText(tr("Search Contacts"));
    connect(ui->searchContactText, &QLineEdit::textChanged, this, &ChatWidget::searchContacts);

    ui->searchContactFilterBox->setMenu(filterMenu);
    updateFilterText();

    connect(filterGroup, &QActionGroup::triggered, this, &ChatWidget::searchContacts);
    connect(filterDisplayGroup, &QActionGroup::triggered, this, &ChatWidget::changeDisplayMode);
}

void ChatWidget::changeDisplayMode() {
    filterDisplayGroup->setEnabled(false);

    //  if (filterDisplayGroup->checkedAction() == filterDisplayActivity) {
    //    contactListWidget->setMode(FriendListWidget::SortingMode::Activity);
    //  } else if (filterDisplayGroup->checkedAction() == filterDisplayName) {
    //    contactListWidget->setMode(FriendListWidget::SortingMode::Name);
    //  }

    searchContacts();
    filterDisplayGroup->setEnabled(true);

    updateFilterText();
}

void ChatWidget::searchContacts() {
    QString searchString = ui->searchContactText->text();

    qDebug() << __func__ << searchString;

    FilterCriteria filter = getFilterCriteria();

    sessionListWidget->searchChatrooms(searchString, filterOnline(filter), filterOffline(filter),
                                       filterGroups(filter));
    sessionListWidget->reDraw();
    updateFilterText();
}

void ChatWidget::updateFilterText() {
    QString action = filterDisplayGroup->checkedAction()->text();
    QString text = filterGroup->checkedAction()->text();
    text = action + QStringLiteral(" | ") + text;
    ui->searchContactFilterBox->setText(text);
}

ChatWidget::FilterCriteria ChatWidget::getFilterCriteria() const {
    QAction* checked = filterGroup->checkedAction();

    if (checked == filterOnlineAction)
        return FilterCriteria::Online;
    else if (checked == filterOfflineAction)
        return FilterCriteria::Offline;
    else if (checked == filterFriendsAction)
        return FilterCriteria::Friends;
    else if (checked == filterGroupsAction)
        return FilterCriteria::Groups;

    return FilterCriteria::All;
}

bool ChatWidget::filterGroups(FilterCriteria index) {
    switch (index) {
        case FilterCriteria::Offline:
        case FilterCriteria::Friends:
            return true;
        default:
            return false;
    }
}

bool ChatWidget::filterOffline(FilterCriteria index) {
    switch (index) {
        case FilterCriteria::Online:
        case FilterCriteria::Groups:
            return true;
        default:
            return false;
    }
}

bool ChatWidget::filterOnline(FilterCriteria index) {
    switch (index) {
        case FilterCriteria::Offline:
        case FilterCriteria::Groups:
            return true;
        default:
            return false;
    }
}

bool ChatWidget::groupsVisible() const {
    FilterCriteria filter = getFilterCriteria();
    return !filterGroups(filter);
}

void ChatWidget::retranslateUi() {
    ui->searchContactText->setPlaceholderText(tr("Search Contacts"));
    ui->retranslateUi(this);

    filterDisplayName->setText(tr("By Name"));
    filterDisplayActivity->setText(tr("By Activity"));
    filterAllAction->setText(tr("All"));
    filterOnlineAction->setText(tr("Online"));
    filterOfflineAction->setText(tr("Offline"));
    filterFriendsAction->setText(tr("Friends"));
    filterGroupsAction->setText(tr("Groups"));

    statusOnline->setText(tr("Online", "Button to set your status to 'Online'"));
    statusAway->setText(tr("Away", "Button to set your status to 'Away'"));
    statusBusy->setText(tr("Busy", "Button to set your status to 'Busy'"));
}

void ChatWidget::setupStatus() {
    int icon_size = 15;

    // Preparing icons and set their size
    statusOnline = new QAction(this);
    statusOnline->setIcon(ok::base::SvgUtils::prepareIcon(Status::getIconPath(Status::Status::Online),
                                                icon_size, icon_size));
    connect(statusOnline, &QAction::triggered, this, &ChatWidget::setStatusOnline);

    statusAway = new QAction(this);
    statusAway->setIcon(
            ok::base::SvgUtils::prepareIcon(Status::getIconPath(Status::Status::Away), icon_size, icon_size));
    connect(statusAway, &QAction::triggered, this, &ChatWidget::setStatusAway);

    statusBusy = new QAction(this);
    statusBusy->setIcon(
            ok::base::SvgUtils::prepareIcon(Status::getIconPath(Status::Status::Busy), icon_size, icon_size));
    connect(statusBusy, &QAction::triggered, this, &ChatWidget::setStatusBusy);

    QMenu* statusButtonMenu = new QMenu(ui->statusButton);
    statusButtonMenu->addAction(statusOnline);
    statusButtonMenu->addAction(statusAway);
    statusButtonMenu->addAction(statusBusy);
    ui->statusButton->setMenu(statusButtonMenu);

    //  ui->searchContactText->setPlaceholderText(tr("Search Contacts"));
    statusOnline->setText(tr("Online", "Button to set your status to 'Online'"));
    statusAway->setText(tr("Away", "Button to set your status to 'Away'"));
    statusBusy->setText(tr("Busy", "Button to set your status to 'Busy'"));
    //  actionLogout->setText(tr("Logout", "Tray action menu to logout user"));
    //  actionQuit->setText(tr("Exit", "Tray action menu to exit tox"));
    //  actionShow->setText(tr("Show", "Tray action menu to show qTox window"));
}

void ChatWidget::cancelFile(const QString& friendId, const QString& fileId) {
    qDebug() << __func__ << "file:" << fileId;
    auto frndId = FriendId{friendId};

    //    Friend *f = FriendList::findFriend(frndId);
    //    if (!f) {
    //        qWarning() <<"IMFriend is no existing!" << friendId;
    //        return;
    //    }

    sessionListWidget->setFriendFileCancelled(frndId, fileId);
}

void ChatWidget::dispatchFile(ToxFile file) {
    qDebug() << __func__ << "file:" << file.toString();

    const auto& cId = ContactId(file.getFriendId());

    if (file.status == FileStatus::INITIALIZING && file.direction == FileDirection::RECEIVING) {
        const Settings& settings = Settings::getInstance();
        //    QString autoAcceptDir = settings.getAutoAcceptDir(cId);
        //    if (autoAcceptDir.isEmpty() &&
        //    ok::base::OkSettings::getInstance().getAutoSaveEnabled()) {
        auto autoAcceptDir = settings.getGlobalAutoAcceptDir();
        //    }

        //    auto maxAutoAcceptSize = settings.getMaxAutoAcceptSize();
        //    bool autoAcceptSizeCheckPassed = maxAutoAcceptSize == 0 || maxAutoAcceptSize >=
        //    file.fileSize;

        //    if (!autoAcceptDir.isEmpty() && autoAcceptSizeCheckPassed) {
        acceptFileTransfer(file, autoAcceptDir);
        //    }
    }

    sessionListWidget->setFriendFileReceived(cId, file);
}

void ChatWidget::dispatchFileWithBool(ToxFile file, bool) { dispatchFile(file); }

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

    core->setStatus(Status::Status::Online);
}

void ChatWidget::setStatusAway() {
    //  if (!ui->statusButton->isEnabled()) {
    //    return;
    //  }

    core->setStatus(Status::Status::Away);
}

void ChatWidget::setStatusBusy() {
    //  if (!ui->statusButton->isEnabled()) {
    //    return;
    //  }

    core->setStatus(Status::Status::Busy);
}

void ChatWidget::onAvInvite(ToxPeer peerId, bool video) {
    qDebug() << __func__ << "friendId" << peerId << video;
    sessionListWidget->setFriendAvInvite(peerId, video);

    //  auto testedFlag = video ?
    //              Settings::AutoAcceptCall::Video : Settings::AutoAcceptCall::Audio;

    //  // AutoAcceptCall is set for this friend
    //  if (Settings::getInstance()
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
    qDebug() << __func__ << "friendId" << friendId;
    sessionListWidget->setFriendAvStart(friendId, video);
}

void ChatWidget::onAvEnd(const FriendId& friendId, bool error) {
    qDebug() << __func__ << "friendId" << friendId << "error" << error;
    sessionListWidget->setFriendAvEnd(friendId, error);

    //  headWidget->removeCallConfirm();
    //  // Fixes an OS X bug with ending a call while in full screen
    //  if (netcam && netcam->isFullScreen()) {
    //    netcam->showNormal();
    //  }

    //  emit stopNotification();
    //  emit endCallNotification();

    //  auto status = Core::getInstance()->getStatus();
    //  updateCallButtons(status);
    //    stopCounter(error);
    //  hideNetcam();
}

void ChatWidget::onFriendNickChanged(const FriendId& friendPk, const QString& nickname) {
    sessionListWidget->setFriendName(friendPk, nickname);
}
void ChatWidget::onFriendAvatarChanged(const FriendId& friendPk, const QByteArray& avatar) {
    sessionListWidget->setFriendAvatar(friendPk, avatar);
}
