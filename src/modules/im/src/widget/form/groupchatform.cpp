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

#include "groupchatform.h"

#include "lib/settings/translator.h"
#include "src/chatlog/chatlog.h"
#include "src/chatlog/content/text.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/core/groupid.h"
#include "src/friendlist.h"
#include "src/lib/settings/style.h"
#include "src/model/friend.h"
#include "src/model/group.h"
#include "src/persistence/igroupsettings.h"
#include "src/persistence/profile.h"
#include "src/video/groupnetcamview.h"
#include "src/widget/chatformheader.h"
#include "src/widget/flowlayout.h"
#include "src/widget/form/chatform.h"
#include "src/widget/groupwidget.h"
#include "src/widget/maskablepixmapwidget.h"
#include "src/widget/tool/croppinglabel.h"
#include "tabcompleter.h"

#include <QDragEnterEvent>
#include <QMimeData>
#include <QRegularExpression>
#include <QTimer>
#include <QToolButton>

#include <src/nexus.h>

namespace {
const auto LABEL_PEER_TYPE_OUR = QVariant(QStringLiteral("our"));
const auto LABEL_PEER_TYPE_MUTED = QVariant(QStringLiteral("muted"));
const auto LABEL_PEER_PLAYING_AUDIO = QVariant(QStringLiteral("true"));
const auto LABEL_PEER_NOT_PLAYING_AUDIO = QVariant(QStringLiteral("false"));
const auto PEER_LABEL_STYLE_SHEET_PATH = QStringLiteral("chatArea/chatHead.css");
}  // namespace

/**
 * @brief Edit name for correct representation if it is needed
 * @param name Editing string
 * @return Source name if it does not contain any newline character, otherwise it chops characters
 * starting with first newline character and appends "..."
 */
QString editName(const QString& name) {
    const int pos = name.indexOf(QRegularExpression(QStringLiteral("[\n\r]")));
    if (pos == -1) {
        return name;
    }

    QString result = name;
    const int len = result.length();
    result.chop(len - pos);
    // TODO: ... result.append(("…")); // \u2026 Unicode symbol, not just three separate dots
    return result;
}

/**
 * @var QList<QLabel*> GroupChatForm::peerLabels
 * @brief Maps peernumbers to the QLabels in namesListLayout.
 *
 * @var QMap<int, QTimer*> GroupChatForm::peerAudioTimers
 * @brief Timeout = peer stopped sending audio.
 */

GroupChatForm::GroupChatForm(const GroupId* chatGroup, IChatLog& chatLog,
                             IMessageDispatcher& messageDispatcher, IGroupSettings& _settings)
        : GenericChatForm(chatGroup, chatLog, messageDispatcher)
        , group(chatGroup)
        , inCall(false)
        , settings(_settings) {
    nusersLabel = new QLabel();

    //    tabber = new TabCompleter(msgEdit, group);

    fileButton->setEnabled(false);
    fileButton->setProperty("state", "");
    ChatFormHeader::Mode mode = ChatFormHeader::Mode::None;
    //    if (group->isAvGroupchat()) {
    //        mode = ChatFormHeader::Mode::Audio;
    //    }

    //    headWidget->setMode(mode);
    //    setName(group->getName());

    nusersLabel->setFont(Style::getFont(Style::Medium));
    nusersLabel->setObjectName("statusLabel");
    retranslateUi();

    //    const QSize& size = headWidget->getAvatarSize();
    //    headWidget->setAvatar(Style::scaleSvgImage(":/img/group_dark.svg", size.width(),
    //    size.height()));

    msgEdit->setObjectName("group");

    //    namesListLayout = new FlowLayout(0, 5, 0);
    //    headWidget->addWidget(nusersLabel);
    //    headWidget->addLayout(namesListLayout);
    //    headWidget->addStretch();

    //    nameLabel->setMinimumHeight(12);
    nusersLabel->setMinimumHeight(12);

    //    connect(msgEdit, &ChatTextEdit::tabPressed, tabber, &TabCompleter::complete);
    //    connect(msgEdit, &ChatTextEdit::keyPressed, tabber, &TabCompleter::reset);

    //    connect(headWidget, &ChatFormHeader::callTriggered, this, &GroupChatForm::onCallClicked);
    //    connect(headWidget, &ChatFormHeader::micMuteToggle, this,
    //    &GroupChatForm::onMicMuteToggle); connect(headWidget, &ChatFormHeader::volMuteToggle,
    //    this, &GroupChatForm::onVolMuteToggle);

    //    connect(headWidget, &ChatFormHeader::nameChanged, chatGroup, &Group::setName);
    //    connect(group, &Group::subjectChanged, this, &GroupChatForm::onTitleChanged);
    //    connect(group, &Group::userJoined, this, &GroupChatForm::onUserJoined);
    //    connect(group, &Group::userLeft, this, &GroupChatForm::onUserLeft);
    //    connect(group, &Group::peerNameChanged, this, &GroupChatForm::onPeerNameChanged);
    //    connect(group, &Group::peerCountChanged, this, &GroupChatForm::updateUserCount);
    settings.connectTo_blackListChanged(this,
                                        [this](QStringList const&) { this->updateUserNames(); });

    //    updateUserNames();
    setAcceptDrops(true);
    settings::Translator::registerHandler(std::bind(&GroupChatForm::retranslateUi, this), this);
}

GroupChatForm::~GroupChatForm() { settings::Translator::unregister(this); }

void GroupChatForm::onTitleChanged(const QString& author, const QString& title) {
    if (author.isEmpty()) {
        return;
    }

    const QString message = tr("%1 has set the title to %2").arg(author, title);
    const QDateTime curTime = QDateTime::currentDateTime();
    addSystemInfoMessage(message, ChatMessage::INFO, curTime);
}

void GroupChatForm::onScreenshotClicked() {
    // Unsupported
}

void GroupChatForm::onAttachClicked() {
    // Unsupported
}

/**
 * @brief Updates user names' labels at the top of group chat
 */
void GroupChatForm::updateUserNames() {
    QLayoutItem* child;
    while ((child = namesListLayout->takeAt(0))) {
        child->widget()->hide();
        delete child->widget();
        delete child;
    }

    peerLabels.clear();
    //    const auto peers = group->getPeerList();
    // no need to do anything without any peers
    //    if (peers.isEmpty()) {
    //        return;
    //    }

    /* we store the peer labels by their ToxPk, but the namelist layout
     * needs it in alphabetical order, so we first create and store the labels
     * and then sort them by their text and add them to the layout in that order */
    const auto selfPk = Core::getInstance()->getSelfId();
    //    for (const auto& peerPk : peers.keys()) {
    //        const QString peerName = peers.value(peerPk);
    //        const QString editedName = editName(peerName);
    //        QLabel* const label = new QLabel(editedName + QLatin1String(", "));
    //        if (editedName != peerName) {
    //            label->setToolTip(peerName + " (" + peerPk + ")");
    //        } else if (peerName != peerPk ) {
    //            label->setToolTip(peerPk );
    //        } // else their name is just their Pk, no tooltip needed
    //        label->setTextFormat(Qt::PlainText);
    //        label->setContextMenuPolicy(Qt::CustomContextMenu);

    //        connect(label, &QLabel::customContextMenuRequested, this,
    //        &GroupChatForm::onLabelContextMenuRequested);

    //        if (peerPk == selfPk.toString()) {
    //            label->setProperty("peerType", LABEL_PEER_TYPE_OUR);
    //        } else if (settings.getBlackList().contains(peerPk)) {
    //            label->setProperty("peerType", LABEL_PEER_TYPE_MUTED);
    //        }

    //        label->setStyleSheet(Style::getStylesheet(PEER_LABEL_STYLE_SHEET_PATH));
    //        peerLabels.insert(peerPk, label);
    //    }

    // add the labels in alphabetical order into the layout
    auto nickLabelList = peerLabels.values();

    std::sort(nickLabelList.begin(), nickLabelList.end(), [](const QLabel* a, const QLabel* b) {
        return a->text().toLower() < b->text().toLower();
    });

    // remove comma from last sorted label
    if (!nickLabelList.isEmpty()) {
        QLabel* const lastLabel = nickLabelList.last();
        QString labelText = lastLabel->text();
        labelText.chop(2);
        lastLabel->setText(labelText);
        for (QLabel* l : nickLabelList) {
            namesListLayout->addWidget(l);
        }
    }
}

void GroupChatForm::onUserJoined(const FriendId& user, const QString& name) {
    if (settings.getShowGroupJoinLeaveMessages()) {
        addSystemInfoMessage(tr("%1 has joined the group").arg(name), ChatMessage::INFO,
                             QDateTime::currentDateTime());
    }
    updateUserNames();
}

void GroupChatForm::onUserLeft(const FriendId& user, const QString& name) {
    if (settings.getShowGroupJoinLeaveMessages()) {
        addSystemInfoMessage(tr("%1 has left the group").arg(name), ChatMessage::INFO,
                             QDateTime::currentDateTime());
    }
    updateUserNames();
}

void GroupChatForm::onPeerNameChanged(const FriendId& peer, const QString& oldName,
                                      const QString& newName) {
    addSystemInfoMessage(tr("%1 is now known as %2").arg(oldName, newName), ChatMessage::INFO,
                         QDateTime::currentDateTime());
    updateUserNames();
}

void GroupChatForm::peerAudioPlaying(QString peerPk) {
    peerLabels[peerPk]->setProperty("playingAudio", LABEL_PEER_PLAYING_AUDIO);
    peerLabels[peerPk]->style()->unpolish(peerLabels[peerPk]);
    peerLabels[peerPk]->style()->polish(peerLabels[peerPk]);
    // TODO(sudden6): check if this can ever be false, cause [] default constructs
    if (!peerAudioTimers[peerPk]) {
        peerAudioTimers[peerPk] = new QTimer(this);
        peerAudioTimers[peerPk]->setSingleShot(true);
        connect(peerAudioTimers[peerPk], &QTimer::timeout, [this, peerPk] {
            auto it = peerLabels.find(peerPk);
            if (it != peerLabels.end()) {
                peerLabels[peerPk]->setProperty("playingAudio", LABEL_PEER_NOT_PLAYING_AUDIO);
                peerLabels[peerPk]->style()->unpolish(peerLabels[peerPk]);
                peerLabels[peerPk]->style()->polish(peerLabels[peerPk]);
            }
            delete peerAudioTimers[peerPk];
            peerAudioTimers[peerPk] = nullptr;
        });
    }

    peerLabels[peerPk]->setStyleSheet(Style::getStylesheet(PEER_LABEL_STYLE_SHEET_PATH));
    peerAudioTimers[peerPk]->start(500);
}

void GroupChatForm::dragEnterEvent(QDragEnterEvent* ev) {
    if (!ev->mimeData()->hasFormat("toxPk")) {
        return;
    }
    FriendId toxPk{ev->mimeData()->data("toxPk")};

    auto profile = Nexus::getProfile();
    if (profile) {
        Friend* frnd = Nexus::getCore()->getFriendList().findFriend(toxPk);
        if (frnd) ev->acceptProposedAction();
    }
}

void GroupChatForm::dropEvent(QDropEvent* ev) {
    if (!ev->mimeData()->hasFormat("toxPk")) {
        return;
    }
    FriendId toxPk{ev->mimeData()->data("toxPk")};
    auto profile = Nexus::getProfile();
    if (!profile) {
        return;
    }

    Friend* frnd = Nexus::getCore()->getFriendList().findFriend(toxPk);
    if (!frnd) return;

    QString friendId = frnd->getId().toString();
    QString cid = contactId->toString();
    if (Status::isOnline(frnd->getStatus())) {
        //        Core::getInstance()->groupInviteFriend(friendId, cid);
    }
}

void GroupChatForm::onMicMuteToggle() {
    if (audioInputFlag) {
        //        CoreAV* av = Core::getInstance()->getAv();
        //        const bool oldMuteState = av->isGroupCallInputMuted(group);
        //        const bool newMute = !oldMuteState;
        //        av->muteCallInput(group, newMute);
        //        headWidget->updateMuteMicButton(inCall, newMute);
    }
}

void GroupChatForm::onVolMuteToggle() {
    if (audioOutputFlag) {
        //        CoreAV* av = Core::getInstance()->getAv();
        //        const bool oldMuteState = av->isGroupCallOutputMuted(group);
        //        const bool newMute = !oldMuteState;
        //        av->muteCallOutput(group, newMute);
        //        headWidget->updateMuteVolButton(inCall, newMute);
    }
}

void GroupChatForm::onCallClicked() {
    if (!inCall) {
        joinGroupCall();
    } else {
        leaveGroupCall();
    }

    //    headWidget->updateCallButtons(true, inCall);

    //    CoreAV* av = Core::getInstance()->getAv();
    //    const bool inMute = av->isGroupCallInputMuted(group);
    //    headWidget->updateMuteMicButton(inCall, inMute);

    //    const bool outMute = av->isGroupCallOutputMuted(group);
    //    headWidget->updateMuteVolButton(inCall, outMute);
}

void GroupChatForm::keyPressEvent(QKeyEvent* ev) {
    // Push to talk (CTRL+P)
    if (ev->key() == Qt::Key_P && (ev->modifiers() & Qt::ControlModifier) && inCall) {
        onMicMuteToggle();
    }

    if (msgEdit->hasFocus()) return;
}

void GroupChatForm::keyReleaseEvent(QKeyEvent* ev) {
    // Push to talk (CTRL+P)
    if (ev->key() == Qt::Key_P && (ev->modifiers() & Qt::ControlModifier) && inCall) {
        onMicMuteToggle();
    }

    if (msgEdit->hasFocus()) return;
}

/**
 * @brief Updates users' count label text
 */
void GroupChatForm::updateUserCount(int numPeers) {
    nusersLabel->setText(tr("%n user(s) in chat", "Number of users in chat", numPeers));
    //    headWidget->updateCallButtons(true, inCall);
}

void GroupChatForm::retranslateUi() {
    //    updateUserCount(group->getPeersCount());
}

void GroupChatForm::onLabelContextMenuRequested(const QPoint& localPos) {
    QLabel* label = static_cast<QLabel*>(QObject::sender());

    if (label == nullptr) {
        return;
    }

    const QPoint pos = label->mapToGlobal(localPos);
    const QString muteString = tr("mute");
    const QString unmuteString = tr("unmute");
    QStringList blackList = settings.getBlackList();
    QMenu* const contextMenu = new QMenu(this);
    const FriendId selfPk = Core::getInstance()->getSelfId();

    // delete menu after it stops being used
    connect(contextMenu, &QMenu::aboutToHide, contextMenu, &QObject::deleteLater);

    QString peerPk;
    peerPk = peerLabels.key(label);
    if (peerPk.isEmpty() || peerPk == selfPk.toString()) {
        return;
    }

    const bool isPeerBlocked = blackList.contains(peerPk);
    QString menuTitle = label->text();
    if (menuTitle.endsWith(QLatin1String(", "))) {
        menuTitle.chop(2);
    }
    QAction* menuTitleAction = contextMenu->addAction(menuTitle);
    menuTitleAction->setEnabled(false);  // make sure the title is not clickable
    contextMenu->addSeparator();

    const QAction* toggleMuteAction;
    if (isPeerBlocked) {
        toggleMuteAction = contextMenu->addAction(unmuteString);
    } else {
        toggleMuteAction = contextMenu->addAction(muteString);
    }
    contextMenu->setStyleSheet(Style::getStylesheet(PEER_LABEL_STYLE_SHEET_PATH));

    const QAction* selectedItem = contextMenu->exec(pos);
    if (selectedItem == toggleMuteAction) {
        if (isPeerBlocked) {
            const int index = blackList.indexOf(peerPk);
            if (index != -1) {
                blackList.removeAt(index);
            }
        } else {
            blackList << peerPk;
        }

        settings.setBlackList(blackList);
    }
}

void GroupChatForm::joinGroupCall() {
    //    CoreAV* av = Core::getInstance()->getAv();
    //    av->joinGroupCall(*group);
    audioInputFlag = true;
    audioOutputFlag = true;
    inCall = true;
}

void GroupChatForm::leaveGroupCall() {
    //    CoreAV* av = Core::getInstance()->getAv();
    //    av->leaveGroupCall(group->getId());
    audioInputFlag = false;
    audioOutputFlag = false;
    inCall = false;
}
