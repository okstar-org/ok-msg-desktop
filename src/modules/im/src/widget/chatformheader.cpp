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

#include "chatformheader.h"

#include "lib/settings/translator.h"
#include "src/lib/settings/style.h"
#include "src/widget/maskablepixmapwidget.h"
#include "src/widget/tool/callconfirmwidget.h"
#include "src/widget/tool/croppinglabel.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStyle>
#include <QTextDocument>
#include <QToolButton>

#include <src/core/FriendId.h>
#include <src/core/coreav.h>

#include <src/model/contact.h>
#include <src/model/friend.h>

#include <src/friendlist.h>
#include <src/widget/tool/callconfirmwidget.h>
#include "src/nexus.h"
#include "src/persistence/profile.h"

static const QSize AVATAR_SIZE{40, 40};
static const short HEAD_LAYOUT_SPACING = 5;
static const short MIC_BUTTONS_LAYOUT_SPACING = 4;
static const short BUTTONS_LAYOUT_HOR_SPACING = 4;

namespace {
const QString STYLE_PATH = QStringLiteral("chatForm/buttons.css");

const QString STATE_NAME[] = {
        QString{},
        QStringLiteral("green"),
        QStringLiteral("red"),
        QStringLiteral("yellow"),
        QStringLiteral("yellow"),
};

const QString CALL_TOOL_TIP[] = {
        ChatFormHeader::tr("Can't start audio call"), ChatFormHeader::tr("Start audio call"),
        ChatFormHeader::tr("End audio call"),         ChatFormHeader::tr("Cancel audio call"),
        ChatFormHeader::tr("Accept audio call"),
};

const QString VIDEO_TOOL_TIP[] = {
        ChatFormHeader::tr("Can't start video call"), ChatFormHeader::tr("Start video call"),
        ChatFormHeader::tr("End video call"),         ChatFormHeader::tr("Cancel video call"),
        ChatFormHeader::tr("Accept video call"),
};

const QString VOL_TOOL_TIP[] = {
        ChatFormHeader::tr("Sound can be disabled only during a call"),
        ChatFormHeader::tr("Mute call"),
        ChatFormHeader::tr("Unmute call"),
};

const QString MIC_TOOL_TIP[] = {
        ChatFormHeader::tr("Microphone can be muted only during a call"),
        ChatFormHeader::tr("Mute microphone"),
        ChatFormHeader::tr("Unmute microphone"),
};

template <class T, class Fun>
QPushButton* createButton(const QString& name, T* self, Fun onClickSlot) {
    QPushButton* btn = new QPushButton();
    btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    btn->setObjectName(name);
    btn->setStyleSheet(Style::getStylesheet(STYLE_PATH));
    QObject::connect(btn, &QPushButton::clicked, self, onClickSlot);
    return btn;
}

template <class State>
void setStateToolTip(QAbstractButton* btn, State state, const QString toolTip[]) {
    const int index = static_cast<int>(state);
    btn->setToolTip(toolTip[index]);
}

template <class State> void setStateName(QAbstractButton* btn, State state) {
    const int index = static_cast<int>(state);
    btn->setProperty("state", STATE_NAME[index]);
    btn->setEnabled(index != 0);
}
}  // namespace

ChatFormHeader::ChatFormHeader(const ContactId& contactId, QWidget* parent)
        : QWidget(parent)
        , contactId(contactId)
        , mode{Mode::AV}
        , callState{CallButtonState::Disabled}
        , videoState{CallButtonState::Disabled} {
    QHBoxLayout* headLayout = new QHBoxLayout(this);
    headLayout->setContentsMargins(0, 0, 0, 0);
    // 头像
    avatar = new MaskablePixmapWidget(this, AVATAR_SIZE, ":/img/avatar_mask.svg");
    avatar->setObjectName("avatar");
    headLayout->addWidget(avatar);

    // 名称
    nameLabel = new CroppingLabel(this);
    nameLabel->setObjectName("nameLabel");
    nameLabel->setMinimumHeight(Style::getFont(Style::Medium).pixelSize());
    nameLabel->setEditable(true);
    nameLabel->setTextFormat(Qt::PlainText);
    nameLabel->setText(contactId.username);
    connect(nameLabel, &CroppingLabel::editFinished, this, &ChatFormHeader::nameChanged);

    // 状态
    statusLabel = new QLabel(this);
    statusIcon = new QToolButton(this);
    statusIcon->setIconSize(QSize(10, 10));
    statusIcon->setStyleSheet("border:0px solid; padding:0px");
    QHBoxLayout* status_lyt = new QHBoxLayout();
    status_lyt->addWidget(statusIcon);
    status_lyt->addWidget(statusLabel);
    status_lyt->addStretch(1);

    headTextLayout = new QVBoxLayout(this);
    headTextLayout->addWidget(nameLabel);
    headTextLayout->addLayout(status_lyt);
    headTextLayout->addStretch();
    headLayout->addLayout(headTextLayout, 1);

    // 空间
    headLayout->addSpacing(HEAD_LAYOUT_SPACING);

    // 控制按钮
    callButton = createButton("callButton", this, &ChatFormHeader::callTriggered);
    videoButton = createButton("videoButton", this, &ChatFormHeader::videoCallTriggered);
    QHBoxLayout* buttonsLayout = new QHBoxLayout(this);
    buttonsLayout->addWidget(callButton);
    buttonsLayout->addWidget(videoButton);
    buttonsLayout->setSpacing(BUTTONS_LAYOUT_HOR_SPACING);

    headLayout->addLayout(buttonsLayout, 0);

    setLayout(headLayout);

    updateButtonsView();

    statusLabel->setVisible(false);
    statusIcon->setVisible(false);
    // todo: 当时是自己时，FriendList::findFriend会返回空，更新状态、按钮等逻辑不会触发
    FriendId self = Core::getInstance()->getSelfId();
    isSelf = self.toString() == contactId.toString();
    if (!isSelf) {
        setContact(FriendList::findFriend(contactId));
    } else {
        // todo: 接口不统一
        setName(Core::getInstance()->getNick());
        setAvatar(Nexus::getProfile()->loadAvatar(contactId));
        connect(Nexus::getProfile(), &Profile::selfAvatarChanged, this, &ChatFormHeader::setAvatar);
        connect(Core::getInstance(), &Core::usernameSet, this, &ChatFormHeader::setName);
    }

    settings::Translator::registerHandler([this] { retranslateUi(); }, this);
    retranslateUi();
}

ChatFormHeader::~ChatFormHeader() { settings::Translator::unregister(this); }

void ChatFormHeader::setContact(const Contact* contact_) {
    if (!contact_ || contactId != contact_->getPersistentId()) {
        if (Friend* old = FriendList::findFriend(contactId)) {
            old->disconnect(this);
            // 是否有必要？
            if (!old->isGroup()) {
                auto f = static_cast<const Friend*>(old);
                f->disconnect(this);
            }
        }
        // 如果是自己，则通过profile连接更新
        if (contactId.getId() == Core::getInstance()->getSelfId().getId()) {
            Nexus::getProfile()->disconnect(this);
            Core::getInstance()->disconnect(this);
        }
        contactId = ContactId();
        this->contact = nullptr;
    }

    if (!contact_ || this->contact == contact_) {
        return;
    }
    this->contact = contact_;
    contactId = contact_->getPersistentId();
    connect(contact_, &Contact::displayedNameChanged, this,
            &ChatFormHeader::onDisplayedNameChanged);
    connect(contact_, &Contact::avatarChanged, this, &ChatFormHeader::setAvatar);

    setName(contact_->getDisplayedName());
    setAvatar(contact_->getAvatar());

    if (!contact_->isGroup()) {
        auto f = static_cast<const Friend*>(contact_);
        updateCallButtons(f->getStatus());
        updateContactStatus(f->getStatus());

        connect(f, &Friend::statusChanged, this, [this](Status::Status status, bool event) {
            updateCallButtons(status);
            updateContactStatus(status);
        });

        statusLabel->setVisible(true);
        statusIcon->setVisible(true);
    } else {
        statusLabel->setVisible(false);
        statusIcon->setVisible(false);
    }
}

void ChatFormHeader::removeContact() {
    qDebug() << __func__;
    contactId = ContactId();
}

const Contact* ChatFormHeader::getContact() const { return FriendList::findFriend(contactId); }

void ChatFormHeader::setName(const QString& newName) {
    nameLabel->setText(newName);
    // for overlength names
    nameLabel->setToolTip(Qt::convertFromPlainText(newName, Qt::WhiteSpaceNormal));
}

void ChatFormHeader::setMode(ChatFormHeader::Mode mode) {
    this->mode = mode;
    if (mode == Mode::None) {
        callButton->hide();
        videoButton->hide();
    }
}

void ChatFormHeader::retranslateUi() {
    setStateToolTip(callButton, callState, CALL_TOOL_TIP);
    setStateToolTip(videoButton, videoState, VIDEO_TOOL_TIP);
}

void ChatFormHeader::updateButtonsView() {
    callButton->setEnabled(callState != CallButtonState::Disabled);
    videoButton->setEnabled(videoState != CallButtonState::Disabled);
    retranslateUi();
    Style::repolish(this);
}

void ChatFormHeader::onDisplayedNameChanged(const QString& name) { setName(name); }

void ChatFormHeader::nameChanged(const QString& name) {
    if (Core::getInstance()->getSelfId().getId() == contactId.getId()) {
        auto profile = Nexus::getProfile();
        profile->setNick(name, true);
        return;
    }

    if (auto f = FriendList::findFriend(contactId)) {
        f->setAlias(name);
        Core::getInstance()->setFriendAlias(contactId.getId(), name);
    }
}

void ChatFormHeader::updateContactStatus(Status::Status status) {
    auto pix = Status::getIconPath(status, false);
    statusIcon->setIcon(QIcon(pix));
    statusLabel->setText(Status::getTitle(status));
}

void ChatFormHeader::showOutgoingCall(bool video) {
    CallButtonState& state = video ? videoState : callState;
    state = CallButtonState::Outgoing;
    updateButtonsView();
}

void ChatFormHeader::createCallConfirm(const ToxPeer& peer, bool video, QString& displayedName) {
    qDebug() << __func__ << "peer:" << peer << "video?" << video;

    QWidget* btn = video ? videoButton : callButton;
    callConfirm = std::make_unique<CallConfirmWidget>(btn);
    //    callConfirm->move(btn->pos());

    connect(callConfirm.get(), &CallConfirmWidget::accepted, this, [=]() {
        removeCallConfirm();
        emit callAccepted(peer, video);
    });
    connect(callConfirm.get(), &CallConfirmWidget::rejected, this, [=]() {
        removeCallConfirm();
        emit callRejected(peer);
    });
}

void ChatFormHeader::showCallConfirm() {
    callConfirm->show();
    //   callConfirm->setVisible(true);
}

void ChatFormHeader::removeCallConfirm() { callConfirm.reset(nullptr); }

void ChatFormHeader::updateMuteVolButton() {
    const CoreAV* av = CoreAV::getInstance();
    bool active = av->isCallActive(&contactId);
    bool outputMuted = av->isCallOutputMuted(&contactId);
    updateMuteVolButton(active, outputMuted);
    //  if (netcam) {
    //    netcam->updateMuteVolButton(outputMuted);
    //  }
}

void ChatFormHeader::updateMuteMicButton() {
    const CoreAV* av = CoreAV::getInstance();
    bool active = av->isCallActive(&contactId);
    bool inputMuted = av->isCallInputMuted(&contactId);
    updateMuteMicButton(active, inputMuted);
    //  if (netcam) {
    //    netcam->updateMuteMicButton(inputMuted);
    //  }
}

void ChatFormHeader::updateCallButtons(bool online, bool audio, bool video) {
    const bool audioAvaliable = online && (mode & Mode::Audio);
    const bool videoAvaliable = online && (mode & Mode::Video);
    if (!audioAvaliable) {
        callState = CallButtonState::Disabled;
    } else if (video) {
        callState = CallButtonState::Disabled;
    } else if (audio) {
        callState = CallButtonState::InCall;
    } else {
        callState = CallButtonState::Avaliable;
    }

    if (!videoAvaliable) {
        videoState = CallButtonState::Disabled;
    } else if (video) {
        videoState = CallButtonState::InCall;
    } else if (audio) {
        videoState = CallButtonState::Disabled;
    } else {
        videoState = CallButtonState::Avaliable;
    }

    updateButtonsView();
}

void ChatFormHeader::updateMuteMicButton(bool active, bool inputMuted) { updateButtonsView(); }

void ChatFormHeader::updateMuteVolButton(bool active, bool outputMuted) { updateButtonsView(); }

void ChatFormHeader::updateCallButtons() {
    updateMuteMicButton();
    updateMuteVolButton();
}

void ChatFormHeader::updateCallButtons(Status::Status status) {
    //    qDebug() << __func__ << (int)status;
    CoreAV* av = CoreAV::getInstance();
    const bool audio = av->isCallActive(&contactId);
    const bool video = av->isCallVideoEnabled(&contactId);
    const bool online = Status::isOnline(status);

    updateCallButtons(online, audio, video);
    updateCallButtons();
}

void ChatFormHeader::setAvatar(const QPixmap& img) { avatar->setPixmap(img); }

QSize ChatFormHeader::getAvatarSize() const { return QSize{avatar->width(), avatar->height()}; }

void ChatFormHeader::reloadTheme() {
    callButton->setStyleSheet(Style::getStylesheet(STYLE_PATH));
    videoButton->setStyleSheet(Style::getStylesheet(STYLE_PATH));
}

void ChatFormHeader::addWidget(QWidget* widget, int stretch, Qt::Alignment alignment) {
    headTextLayout->addWidget(widget, stretch, alignment);
}

void ChatFormHeader::addLayout(QLayout* layout) { headTextLayout->addLayout(layout); }

void ChatFormHeader::addStretch() { headTextLayout->addStretch(); }

// void ChatFormHeader::updateCallButtons()
//{
//     qDebug() << __func__;
//     updateMuteMicButton();
//     updateMuteVolButton();
// }

// void ChatFormHeader::updateCallButtons(Status::Status status)
//{
//       qDebug() << __func__ << (int)status;

//      CoreAV *av = CoreAV::getInstance();
//      const bool audio = av->isCallActive(contactId);
//      const bool video = av->isCallVideoEnabled(contactId);
//      const bool online = Status::isOnline(status);
//      headWidget->updateCallButtons(online, audio, video);

//      updateCallButtons();
//}

// void ChatFormHeader::updateMuteMicButton() {
//   const CoreAV *av = CoreAV::getInstance();
//   bool active = av->isCallActive(contactId);
//   bool inputMuted = av->isCallInputMuted(contactId);
//   headWidget->updateMuteMicButton(active, inputMuted);
//   if (netcam) {
//     netcam->updateMuteMicButton(inputMuted);
//   }
// }

// void ChatFormHeader::updateMuteVolButton() {
//   const CoreAV *av = CoreAV::getInstance();
//   bool active = av->isCallActive(contactId);
//   bool outputMuted = av->isCallOutputMuted(contactId);
//   headWidget->updateMuteVolButton(active, outputMuted);
//   if (netcam) {
//     netcam->updateMuteVolButton(outputMuted);
//   }
// }
