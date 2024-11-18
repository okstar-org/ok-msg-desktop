#include "MeetingVideoFrame.h"
#include <lib/settings/style.h>
#include "../tools/PopupMenuComboBox.h"
#include "MeetingVideosLayout.h"
#include "base/RoundedPixmapLabel.h"
#include "VideoLayoutPicker.h"

#include <QAction>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>
#include <QWindowStateChangeEvent>
#include <QApplication>

MeetingVideoFrame::MeetingVideoFrame(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground);
    creatTopToolBar();
    creatBottomBar();
    videosLayout = new MeetingVideosLayout(this);
    videosLayout->setObjectName("videoLayout");

    retranslateUi();

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(topToolBar, 0);
    mainLayout->addWidget(videosLayout, 1);
    mainLayout->addWidget(bottomBar, 0);

    updateDuration();
    initConnection();

    reloadTheme();
}

void MeetingVideoFrame::reloadTheme() {
    // 手动拼接一下
    QString style = Style::getStylesheet("MettingBase.css");
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

    if (currType != picker.selectedType())
    {
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
    //QMenu menu(this);
    //menu.addAction("data");
    //audioSettingButton->showMenuOnce(&menu);
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
