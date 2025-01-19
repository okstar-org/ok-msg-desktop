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
#include "../MeetingVideoRender.h"
#include "MeetingVideosLayout.h"
#include "VideoLayoutPicker.h"
#include "application.h"
#include "base/RoundedPixmapLabel.h"
#include "lib/messenger/Messenger.h"
#include "lib/storage/settings/style.h"
#include "lib/ui/widget/tools/PopupMenuComboBox.h"
#include "modules/im/src/core/core.h"
#include "modules/im/src/nexus.h"

#include <QAction>
#include <QApplication>
#include <QElapsedTimer>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include <QWindowStateChangeEvent>

namespace module::meet {

MeetingVideoFrame::MeetingVideoFrame(const QString& name, lib::ortc::CtrlState ctrlState, QWidget* parent)
        : QWidget(parent), username(name),
        duration(0,0,0), // 初始化时间为 00:00:00
        callDurationTimer(nullptr),
        timeElapsed(nullptr),
        ctrlState(ctrlState) {
    setAttribute(Qt::WA_StyledBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    creatTopToolBar();
    creatBottomBar();

    videosLayout = new MeetingVideosContainer(this);
    videosLayout->setObjectName("videoLayout");

    connect(this, &MeetingVideoFrame::participantJoined, this, &MeetingVideoFrame::addParticipant);
    connect(this, &MeetingVideoFrame::participantLeft, this, &MeetingVideoFrame::removeParticipant);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(topToolBar, 0);
    mainLayout->addWidget(videosLayout, 1);
    mainLayout->addWidget(bottomBar, 0);

    initConnection();
    reloadTheme();
    retranslateUi();

    auto profile = ok::Application::Instance()->getProfile();
    meet = new lib::messenger::MessengerMeet(profile->getMessenger());
    meet->addHandler(this);
    createMeet(name);

    callDurationTimer = new QTimer(this);
    connect(callDurationTimer, &QTimer::timeout, this, &MeetingVideoFrame::updateDuration);

    syncAudioVideoState();
}

MeetingVideoFrame::~MeetingVideoFrame() {
    std::lock_guard<std::mutex> g(prt_mutex);

    disconnect(this);
    meet->removeHandler(this);
    delete meet;

    if (timeElapsed != nullptr) {
        delete timeElapsed;
        timeElapsed = nullptr;
    }

    if (!participantMap.isEmpty()) {
        videosLayout->clearParticipant();
        qDeleteAll(participantMap);
        participantMap.clear();
    }
}

void MeetingVideoFrame::reloadTheme() {
    // 手动拼接一下
    QString style = lib::settings::Style::getStylesheet("MeetingBase.css");
    QString style2 = lib::settings::Style::getStylesheet("MeetingVideo.css");
    this->setStyleSheet(style + "\n" + style2);
    QCoreApplication::postEvent(this->topToolBar, new QEvent(QEvent::StyleChange));
}

void MeetingVideoFrame::creatTopToolBar() {
    topToolBar = new QToolBar(this);
    topToolBar->setObjectName("topBar");
    topToolBar->setIconSize(QSize(16, 16));
    infoAction = topToolBar->addAction(tr("Meeting Info"));

    // 网络图标
    netInfoAction = topToolBar->addAction(QIcon(":/meet/image/network_s4.svg"), QString());
    topToolBar->addSeparator();

    // 时长
    duraionLabel = new QLabel("00:00:00", topToolBar);
    topToolBar->addWidget(duraionLabel);
    topToolBar->addSeparator();

    QWidget* stretch = new QWidget(topToolBar);
    stretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    topToolBar->addWidget(stretch);

    sharedAction = topToolBar->addAction(QIcon(":/meet/image/share.svg"), QString());
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
    // audioSettingButton->iconButton()->setIcon(QIcon(":/meet/image/micphone.svg"));
    audioSettingButton->setCursor(Qt::PointingHandCursor);

    connect(audioSettingButton->iconButton(), &QToolButton::clicked, [&](bool checked) {
        ctrlState.enableMic = !ctrlState.enableMic;
        syncAudioVideoState();
    });

    videoSettingButton = new PopupMenuComboBox(bottomBar);
    // videoSettingButton->iconButton()->setIcon(QIcon(":/meet/image/videocam.svg"));
    videoSettingButton->setCursor(Qt::PointingHandCursor);

    connect(videoSettingButton->iconButton(), &QToolButton::clicked, [&](bool checked) {
        ctrlState.enableCam = !ctrlState.enableCam;
        syncAudioVideoState();
    });

    sharedDeskButton = new PopupMenuComboBox(bottomBar);
    sharedDeskButton->iconButton()->setIcon(QIcon(":/meet/image/share_screen.svg"));
    sharedDeskButton->iconButton()->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    recoardButton = new PopupMenuComboBox(bottomBar);
    recoardButton->iconButton()->setIcon(QIcon(":/meet/image/record.svg"));
    inviteButton = new PopupMenuComboBox(bottomBar);
    inviteButton->iconButton()->setIcon(QIcon(":/meet/image/invite_user.svg"));

    leaveButton = new QToolButton(bottomBar);
    leaveButton->setObjectName("leaveMeeting");
    leaveButton->setIcon(QIcon(":/meet/image/phone.svg"));
    connect(leaveButton, &QToolButton::clicked, this, &MeetingVideoFrame::doLeaveMeet);

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

    if (this->windowState() == Qt::WindowFullScreen) {
        fullScreenAction->setToolTip(tr("Exit fullscreen"));
    } else {
        fullScreenAction->setToolTip(tr("Show fullscreen"));
    }
    leaveButton->setToolTip(tr("Leave meeting"));

    recoardButton->setToolTip(tr("Record"));
}

void MeetingVideoFrame::startCounter() {
    // 启动计时
    if (!timeElapsed) {
        timeElapsed = new QElapsedTimer();
    }
    timeElapsed->start();

    if (!timeElapsed->isValid()) {
        qWarning() << "Unsupported QElapsedTimer!";
        return;
    }

    // 启动计时器
    callDurationTimer->start(1000);
}

void MeetingVideoFrame::stopCounter() {
    if (!timeElapsed || !timeElapsed->isValid()) {
        return;
    }
    timeElapsed->invalidate();

    delete timeElapsed;
    timeElapsed = nullptr;

    callDurationTimer->stop();
}

/**
 * 创建会议
 * @param name
 */
void MeetingVideoFrame::createMeet(const QString& name) {
    qDebug() << __func__ << name;
    meet->create(stdstring(name));
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

void MeetingVideoFrame::onParticipantLeft(const ok::base::Jid& jid,
                                          const std::string& participant) {
    emit participantLeft(jid.node(), qstring(participant));
}

void MeetingVideoFrame::addParticipant(const QString& name,
                                       const lib::messenger::Participant& parti) {
    qDebug() << __func__ << "room:" << name << "email:" << parti.email.c_str()
             << "resource:" << parti.resource.c_str();

    MeetingParticipant* participant;

    auto k = qstring(parti.resource);
    auto find = participantMap.find(k);
    if (find != participantMap.end()) {
        participant = find.value();
    } else {
        // 添加用户
        std::lock_guard<std::mutex> g(prt_mutex);
        participant = new MeetingParticipant(qstring(parti.resource),
                                             qstring(parti.email),
                                             qstring(parti.nick),
                                             (parti.avatarUrl),
                                             ok::base::Jid(parti.jid.c_str()));
        videosLayout->addParticipant(participant);
        participantMap.insert(k, participant);
    }

    // 更新信息
    participant->setNick(qstring(parti.nick));
    participant->setAvatarUrl(parti.avatarUrl);

    // TODO 音频视频禁止信息 parti.sourceInfo;
}

void MeetingVideoFrame::removeParticipant(const QString& name, const QString& resource) {
    qDebug() << __func__ << "participant:" << resource;
    // 执行移除用户操作
    auto it = participantMap.find(resource);
    if (it != participantMap.end()) {
        std::lock_guard<std::mutex> g(prt_mutex);
        auto user = it.value();
        Q_ASSERT(user);
        videosLayout->removeParticipant(user);
        delete user;
        participantMap.erase(it);
    }

    if (participantMap.isEmpty()) {
        this->close();
        this->deleteLater();
    }
}

void MeetingVideoFrame::syncAudioVideoState() {
    if (ctrlState.enableMic) {
        audioSettingButton->iconButton()->setIcon(QIcon(":/meet/image/micphone.svg"));
    } else {
        audioSettingButton->iconButton()->setIcon(QIcon(":/meet/image/micphone_mute.svg"));
    }

    if (ctrlState.enableCam) {
        videoSettingButton->iconButton()->setIcon(QIcon(":/meet/image/videocam.svg"));
    } else {
        videoSettingButton->iconButton()->setIcon(QIcon(":/meet/image/videocam_stop.svg"));
    }
    // 设置会议音视频开启和关闭
    meet->setCtrlState(ctrlState);
}

void MeetingVideoFrame::doLeaveMeet() {
    meet->leave();
    emit meetLeft();
}

void MeetingVideoFrame::updateDuration() {
    if (!timeElapsed) {
        return;
    }
    // 获取已经流逝的时间（以秒为单位）
    auto elapsedSeconds = timeElapsed->elapsed() / 1000;

    // 将秒转换为 HH:MM:SS 格式
    duration = duration.addSecs(elapsedSeconds);

    // 更新标签文本
    duraionLabel->setText(duration.toString("hh:mm:ss"));
}

void MeetingVideoFrame::onParticipantVideoFrame(const std::string& participant,
                                                const lib::ortc::RendererImage& image) {
    std::lock_guard<std::mutex> g(prt_mutex);
    for (auto it = participantMap.begin(); it != participantMap.end(); it++) {
        MeetingParticipant* p = *it;
        if (stdstring(p->getResource()) == participant) {
            p->videoRender()->renderImage(image);
        }
    }
}

void MeetingVideoFrame::onParticipantMessage(const std::string& participant,
                                             const std::string& msg) {
    // TODO 处理成员消息
    qDebug() << __func__ << "message coming: " << msg.c_str() << " from: " << participant.c_str();
}

void MeetingVideoFrame::onEnd() {
    qDebug() << __func__;
    emit meetDestroyed();
}

}  // namespace module::meet
