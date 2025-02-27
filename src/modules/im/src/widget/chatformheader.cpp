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

#include "lib/storage/settings/style.h"
#include "lib/storage/settings/translator.h"
#include "lib/ui/widget/tools/CroppingLabel.h"
#include "lib/ui/widget/tools/MaskablePixmap.h"
#include "src/widget/tool/callconfirmwidget.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>
#include <QStyle>
#include <QTextDocument>
#include <QToolButton>

#include "src/core/coreav.h"
#include "src/model/FriendId.h"
#include "src/model/contact.h"
#include "src/model/friend.h"
#include "src/model/friendlist.h"
#include "src/widget/tool/callconfirmwidget.h"
#include "Bus.h"
#include "application.h"
#include "base/Styles.h"
#include "src/persistence/profile.h"
#include "src/nexus.h"
#include "widget.h"

namespace module::im {

static const short HEAD_LAYOUT_SPACING = 5;
static const short MIC_BUTTONS_LAYOUT_SPACING = 4;
static const short BUTTONS_LAYOUT_HOR_SPACING = 4;

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
    btn->setStyleSheet(lib::settings::Style::getStylesheet(STYLE_PATH));
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

ChatFormHeader::ChatFormHeader(const ContactId& contactId, QWidget* parent)
        : QWidget(parent)
        , contactId(contactId)
        , mode{Mode::AV}
        , callState{CallButtonState::Disabled}
        , videoState{CallButtonState::Disabled}
        , mProfile{nullptr} {
    QHBoxLayout* headLayout = new QHBoxLayout(this);
    headLayout->setContentsMargins(0, 0, 0, 0);
    // 头像
    avatar = new lib::ui::MaskablePixmapWidget(this, base::Styles::AVATAR_SIZE,
                                               ":/img/avatar_mask.svg");
    avatar->setObjectName("avatar");
    headLayout->addWidget(avatar);

    // 名称
    nameLabel = new lib::ui::CroppingLabel(this);
    nameLabel->setObjectName("nameLabel");
    nameLabel->setMinimumHeight(
            lib::settings::Style::getFont(lib::settings::Style::Font::Medium).pixelSize());
    nameLabel->setCursor(Qt::PointingHandCursor);
    nameLabel->setTextFormat(Qt::PlainText);
    nameLabel->setText(contactId.username);

    connect(nameLabel, &lib::ui::CroppingLabel::clicked, [&]() {
        auto w = Widget::getInstance();
        w->toShowDetails(contactId);
    });

    //    nameLabel->setEditable(true);
    //    connect(nameLabel, &CroppingLabel::editFinished, this, &ChatFormHeader::nameChanged);

    // 状态
    statusLabel = new QLabel(this);
    statusIcon = new QToolButton(this);
    statusIcon->setIconSize(QSize(10, 10));
    statusIcon->setStyleSheet("border:0px solid; padding:0px");
    QHBoxLayout* status_lyt = new QHBoxLayout(this);
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

    mProfile = Nexus::getProfile();
    connect(ok::Application::Instance()->bus(), &ok::Bus::profileChanged, this,
            [&](Profile* profile) { mProfile = profile; });

    setContact(Nexus::getCore()->getFriendList().findFriend(contactId));


    retranslateUi();
    auto a = ok::Application::Instance();
    connect(a->bus(), &ok::Bus::languageChanged,this,
            [&](const QString& locale0) {
                retranslateUi();
            });
}

ChatFormHeader::~ChatFormHeader() {
}

void ChatFormHeader::setContact(const Contact* contact_) {
    if (!Nexus::getProfile() || contact == contact_) {
        return;
    }
    if (contact) {
        contact->disconnect(this);
    }

    contact = contact_;
    if (!contact) {
        qDebug() << "Remove contact";
        return;
    }

    connect(contact_, &Contact::displayedNameChanged, this,
            &ChatFormHeader::onDisplayedNameChanged);
    connect(contact_, &Contact::avatarChanged, this, &ChatFormHeader::setAvatar);

    setName(contact_->getDisplayedName());
    setAvatar(contact_->getAvatar());

    if (!contact_->isGroup()) {
        auto f = static_cast<const Friend*>(contact_);
        updateCallButtons(f->getStatus());
        updateContactStatus(f->getStatus());

        connect(f, &Friend::statusChanged, this, [this](Status status, bool event) {
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

const Contact* ChatFormHeader::getContact() const {
    return contact;
}

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

    if (contact && !contact->isGroup()) {
        auto f = static_cast<const Friend*>(contact);
        statusLabel->setText(getTitle(f->getStatus()));
    }
}

void ChatFormHeader::updateButtonsView() {
    // callButton->setEnabled(callState != CallButtonState::Disabled);
    // videoButton->setEnabled(videoState != CallButtonState::Disabled);
    retranslateUi();
    base::Styles::repolish(this);
}

void ChatFormHeader::onDisplayedNameChanged(const QString& name) {
    setName(name);
}

void ChatFormHeader::nameChanged(const QString& name) {
    if (Core::getInstance()->getSelfId().getId() == contactId.getId()) {
        auto profile = Nexus::getProfile();
        profile->setNick(name, true);
        return;
    }

    if (auto f = Nexus::getCore()->getFriendList().findFriend(contactId)) {
        f->setAlias(name);
        Core::getInstance()->setFriendAlias(contactId.getId(), name);
    }
}

void ChatFormHeader::updateContactStatus(Status status) {
    auto pix = getIconPath(status, false);
    statusIcon->setIcon(QIcon(pix));
    statusLabel->setText(getTitle(status));
}

void ChatFormHeader::showOutgoingCall(bool video) {
    CallButtonState& state = video ? videoState : callState;
    state = CallButtonState::Outgoing;
    updateButtonsView();
}

CallConfirmWidget* ChatFormHeader::createCallConfirm(const PeerId& peer, bool video,
                                                     QString& displayedName) {
    qDebug() << __func__ << "peer:" << peer.toString() << "video?" << video;
    callConfirm = std::make_unique<CallConfirmWidget>(peer, video);
    connect(callConfirm.get(), &CallConfirmWidget::accepted, this, [=]() {
        emit callAccepted(peer, video);
    });
    connect(callConfirm.get(), &CallConfirmWidget::rejected, this, [=]() {
        emit callRejected(peer);
    });
    return callConfirm.get();
}

void ChatFormHeader::showCallConfirm() {
    callConfirm->show();
}

void ChatFormHeader::removeCallConfirm() {
    callConfirm.reset(nullptr);
}

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

void ChatFormHeader::updateMuteMicButton(bool active, bool inputMuted) {
    updateButtonsView();
}

void ChatFormHeader::updateMuteVolButton(bool active, bool outputMuted) {
    updateButtonsView();
}

void ChatFormHeader::updateCallButtons() {
    updateMuteMicButton();
    updateMuteVolButton();
}

void ChatFormHeader::updateCallButtons(Status status) {
    //    qDebug() << __func__ << (int)status;
    CoreAV* av = CoreAV::getInstance();
    const bool audio = av->isCallActive(&contactId);
    const bool video = av->isCallVideoEnabled(&contactId);
    const bool online = isOnline(status);

    updateCallButtons(online, audio, video);
    updateCallButtons();
}

void ChatFormHeader::setAvatar(const QPixmap& img) {
    avatar->setPixmap(img);
}

QSize ChatFormHeader::getAvatarSize() const {
    return QSize{avatar->width(), avatar->height()};
}

void ChatFormHeader::reloadTheme() {
    callButton->setStyleSheet(lib::settings::Style::getStylesheet(STYLE_PATH));
    videoButton->setStyleSheet(lib::settings::Style::getStylesheet(STYLE_PATH));
}

void ChatFormHeader::addWidget(QWidget* widget, int stretch, Qt::Alignment alignment) {
    headTextLayout->addWidget(widget, stretch, alignment);
}

void ChatFormHeader::addLayout(QLayout* layout) {
    headTextLayout->addLayout(layout);
}

void ChatFormHeader::addStretch() {
    headTextLayout->addStretch();
}

// void ChatFormHeader::updateCallButtons()
//{
//     qDebug() << __func__;
//     updateMuteMicButton();
//     updateMuteVolButton();
// }

// void ChatFormHeader::updateCallButtons(Status status)
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
}  // namespace module::im
