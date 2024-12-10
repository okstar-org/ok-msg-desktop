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

#include "MeetingVideoFrame.h"
#include "../MeetingParticipant.h"
#include "../tools/PopupMenuComboBox.h"
#include "MeetingVideosLayout.h"
#include "VideoLayoutPicker.h"
#include "base/RoundedPixmapLabel.h"
#include "lib/messenger/messenger.h"
#include "lib/settings/style.h"
#include "modules/im/src/core/core.h"
#include "modules/im/src/nexus.h"

#include <QAction>
#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include <QWindowStateChangeEvent>
#include <memory>
namespace module::meet {

MeetingVideoFrame::MeetingVideoFrame(const QString& name, QWidget* parent)
        : QWidget(parent), username(name) {
    setAttribute(Qt::WA_StyledBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    creatTopToolBar();
    creatBottomBar();

    videosLayout = new MeetingVideosContainer(this);
    videosLayout->setObjectName("videoLayout");

    connect(this, &MeetingVideoFrame::participantJoined, this, &MeetingVideoFrame::addParticipant);
    connect(this, &MeetingVideoFrame::participantLeft, this, &MeetingVideoFrame::removeParticipant);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(topToolBar, 0);
    mainLayout->addWidget(videosLayout, 1);
    mainLayout->addWidget(bottomBar, 0);

    updateDuration();
    initConnection();
    reloadTheme();
    retranslateUi();

    Core* core = Nexus::getInstance().getCore();
    meet = new lib::messenger::MessengerMeet(core->getMessenger(), this);
    meet->addHandler(this);
    createMeet(name);
}

MeetingVideoFrame::~MeetingVideoFrame() {
    disconnect(this);
    meet->deleteLater();

    if (!participantMap.isEmpty()) {
        videosLayout->clearParticipant();
        qDeleteAll(participantMap);
        participantMap.clear();
    }
}

void MeetingVideoFrame::reloadTheme() {
    // 手动拼接一下
    QString style = Style::getStylesheet("MeetingBase.css");
    QString style2 = Style::getStylesheet("MeetingVideo.css");
    this->setStyleSheet(style + "\n" + style2);
    QCoreApplication::postEvent(this->topToolBar, new QEvent(QEvent::StyleChange));
}

void MeetingVideoFrame::creatTopToolBar() {
    topToolBar = new QToolBar(this);
    topToolBar->setObjectName("topBar");
    topToolBar->setIconSize(QSize(16, 16));
    infoAction = topToolBar->addAction(tr("Meeting Info"));
    sharedAction = topToolBar->addAction(QIcon(":/meet/image/share.svg"), QString());
    topToolBar->addSeparator();

    // 时长
    duraionLabel = new QLabel("00:00:00", topToolBar);
    topToolBar->addWidget(duraionLabel);
    topToolBar->addSeparator();

    netInfoAction = topToolBar->addAction(QIcon(":/meet/image/network_s4.svg"), QString());

    QWidget* stretch = new QWidget(topToolBar);
    stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    topToolBar->addWidget(stretch);

    layoutAction = topToolBar->addAction(QIcon(":/meet/image/layout_grid.svg"), QString());
    fullScreenAction = topToolBar->addAction(QIcon(":/meet/image/fullscreen.svg"), QString());

    connect(fullScreenAction, &QAction::triggered, this, &MeetingVideoFrame::toggleFullScreen);
    connect(layoutAction, &QAction::triggered, this, &MeetingVideoFrame::showLayoutPicker);
}

void MeetingVideoFrame::creatBottomBar() {
    bottomBar = new QFrame(this);
    bottomBar->setObjectName("bottomBar");

    // 左侧部分
    QHBoxLayout* leftLayout = new QHBoxLayout();
    msgButton = new QToolButton(bottomBar);
    msgButton->setIcon(QIcon(":/meet/image/message.svg"));
    leftLayout->addWidget(msgButton);
    leftLayout->addStretch(1);

    // 中间部分
    QHBoxLayout* middleLayout = new QHBoxLayout();
    audioSettingButton = new PopupMenuComboBox(bottomBar);
    audioSettingButton->iconButton()->setIcon(QIcon(":/meet/image/micphone.svg"));
    videoSettingButton = new PopupMenuComboBox(bottomBar);
    videoSettingButton->iconButton()->setIcon(QIcon(":/meet/image/videocam.svg"));
    sharedDeskButton = new PopupMenuComboBox(bottomBar);
    sharedDeskButton->iconButton()->setIcon(QIcon(":/meet/image/share_screen.svg"));
    sharedDeskButton->iconButton()->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    recoardButton = new PopupMenuComboBox(bottomBar);
    recoardButton->iconButton()->setIcon(QIcon(":/meet/image/record.svg"));
    inviteButton = new PopupMenuComboBox(bottomBar);
    inviteButton->iconButton()->setIcon(QIcon(":/meet/image/invite_user.svg"));
    leaveButton = new QToolButton(bottomBar);
    leaveButton->setObjectName("leaveMeeting");
    leaveButton->setIcon(QIcon(":/meet/image/phone"));

    middleLayout->addWidget(audioSettingButton);
    middleLayout->addWidget(videoSettingButton);
    middleLayout->addWidget(sharedDeskButton);
    middleLayout->addWidget(recoardButton);
    middleLayout->addWidget(inviteButton);
    middleLayout->addWidget(leaveButton);

    // 右侧部分
    QHBoxLayout* rightLayout = new QHBoxLayout();
    securityButton = new QToolButton(bottomBar);
    securityButton->setIcon(QIcon(":/meet/image/security.svg"));
    moreOptionButon = new QToolButton(bottomBar);
    moreOptionButon->setIcon(QIcon(":/meet/image/more.svg"));
    rightLayout->addStretch(1);
    rightLayout->addWidget(securityButton);
    rightLayout->addWidget(moreOptionButon);

    QHBoxLayout* barLayout = new QHBoxLayout(bottomBar);
    barLayout->setContentsMargins(0, 0, 0, 0);
    barLayout->addLayout(leftLayout, 1);
    barLayout->addLayout(middleLayout, 0);
    barLayout->addLayout(rightLayout, 1);
}

void MeetingVideoFrame::initConnection() {
    connect(audioSettingButton, &PopupMenuComboBox::menuRequest, this,
            &MeetingVideoFrame::showAudioPopMenu);
}

void MeetingVideoFrame::showLayoutPicker() {
    VideoLayoutPicker picker(this);

    auto currType = videosLayout->currentLayoutType();
    picker.setCurrentType(currType);
    QRect rect = topToolBar->actionGeometry(layoutAction);
    QPoint basePos = topToolBar->mapToGlobal(rect.bottomLeft());
    QSize size = picker.sizeHint();
    basePos.rx() -= (size.width() - rect.width()) / 2;
    picker.exec(basePos);

    if (currType != picker.selectedType()) {
        videosLayout->resetLayout(picker.selectedType());
    }
}

void MeetingVideoFrame::toggleFullScreen() {
    if (this->isFullScreen()) {
        this->showNormal();
    } else {
        this->showFullScreen();
    }
}

void MeetingVideoFrame::updateDuration() {
    // duraionLabel->setText("00:00:00");
    //
}

void MeetingVideoFrame::showAudioPopMenu() {
    // create menu everytime or create once and set menu
    // QMenu menu(this);
    // menu.addAction("data");
    // audioSettingButton->showMenuOnce(&menu);
}

void MeetingVideoFrame::changeEvent(QEvent* event) {
    QWidget::changeEvent(event);
    // 最大化状态变化，更新图标
    if (event->type() == QEvent::WindowStateChange && fullScreenAction) {
        if (this->windowState() == Qt::WindowFullScreen)
            fullScreenAction->setIcon(QIcon(":/meet/image/exit_fullscreen.svg"));
        else
            fullScreenAction->setIcon(QIcon(":/meet/image/fullscreen.svg"));
    }
}

void MeetingVideoFrame::retranslateUi() {
    infoAction->setText(tr("Meeting Info"));
    layoutAction->setText(tr("Layout"));
    sharedAction->setToolTip(tr("Share Meeting"));
    securityButton->setText(tr("Security"));
    sharedDeskButton->iconButton()->setText(tr("Share"));
}

/**
 * 创建会议
 * @param name
 */
void MeetingVideoFrame::createMeet(const QString& name) {
    qDebug() << __func__ << name;
    meet->create(name);
}

void MeetingVideoFrame::onMeetCreated(const ok::base::Jid& jid,
                                      bool ready,
                                      const std::map<std::string, std::string>& props) {
    emit meetCreated(jid.node());
}

void MeetingVideoFrame::onParticipantJoined(const ok::base::Jid& jid,
                                            const lib::messenger::Participant& part) {
    emit participantJoined(jid.node(), part);
}

void MeetingVideoFrame::onParticipantLeft(const ok::base::Jid& jid, const ok::base::Jid& part) {
    emit participantLeft(jid.node(), part);
}

void MeetingVideoFrame::addParticipant(const QString& name,
                                       const lib::messenger::Participant& parti) {
    auto find = participantMap.find(parti.email);
    if (find == participantMap.end()) {
        // 添加用户
        MeetingParticipant p(parti.email, parti.nick, parti.avatarUrl, parti.jid);
        auto user = new MeetingUser(p);

        videosLayout->addParticipant(user);
        participantMap.insert(parti.email, user);
    } else {
        // 更新信息
        auto user = find.value();
        user->setNick(parti.nick);
        user->setAvatarUrl(parti.avatarUrl);
    }
}

void MeetingVideoFrame::removeParticipant(const QString&, const ok::base::Jid& jid) {
    QString email;
    for (auto user : participantMap) {
        /**
         * 移除用户的resource
         */
        auto resCount = user->removeResource(jid.resource());
        if (resCount > 0) {
            // 存在resource（则代表存在其他终端）则不处理
            return;
        }
        // 需要移除的用户
        email = user->getEmail();
    }
    if (email.isEmpty()) return;

    // 执行移除用户操作
    auto itor = participantMap.find(email);
    if (itor != participantMap.end()) {
        auto user = itor.value();
        Q_ASSERT(user);
        videosLayout->removeParticipant(user);
        delete user;
        participantMap.erase(itor);
    }

    if (participantMap.isEmpty()) {
        this->close();
        this->deleteLater();
    }
}

}  // namespace module::meet