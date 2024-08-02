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

#include "genericnetcamview.h"

#include <QApplication>
#include <QBoxLayout>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QPushButton>
#include <QScreen>
#include <QVariant>

namespace {
const auto BTN_STATE_NONE = QVariant("none");
const auto BTN_STATE_RED = QVariant("red");
const int BTN_PANEL_HEIGHT = 55;
const int BTN_PANEL_WIDTH = 250;
const auto BTN_STYLE_SHEET_PATH = QStringLiteral("chatForm/fullScreenButtons.css");
}  // namespace

GenericNetCamView::GenericNetCamView(QWidget* parent) : QWidget(parent) {
    verLayout = new QVBoxLayout(this);
    setWindowTitle(tr("Tox video"));

    buttonLayout = new QHBoxLayout();

    toggleMessagesButton = new QPushButton();
    enterFullScreenButton = new QPushButton();
    enterFullScreenButton->setText(tr("Full Screen"));

    buttonLayout->addStretch();
    buttonLayout->addWidget(toggleMessagesButton);
    buttonLayout->addWidget(enterFullScreenButton);

    connect(toggleMessagesButton, &QPushButton::clicked, this,
            &GenericNetCamView::showMessageClicked);
    connect(enterFullScreenButton, &QPushButton::clicked, this,
            &GenericNetCamView::toggleFullScreen);

    verLayout->addLayout(buttonLayout);
    verLayout->setContentsMargins(0, 0, 0, 0);

    setShowMessages(false);

    setStyleSheet("NetCamView { background-color: #c1c1c1; }");
    buttonPanel = new QFrame(this);
    buttonPanel->setStyleSheet(Style::getStylesheet(BTN_STYLE_SHEET_PATH));
    buttonPanel->setGeometry(0, 0, BTN_PANEL_WIDTH, BTN_PANEL_HEIGHT);

    QHBoxLayout* buttonPanelLayout = new QHBoxLayout(buttonPanel);
    buttonPanelLayout->setContentsMargins(20, 0, 20, 0);

    videoPreviewButton = createButton("videoPreviewButton", "none");
    videoPreviewButton->setToolTip(tr("Toggle video preview"));

    volumeButton = createButton("volButtonFullScreen", "none");
    volumeButton->setToolTip(tr("Mute audio"));

    microphoneButton = createButton("micButtonFullScreen", "none");
    microphoneButton->setToolTip(tr("Mute microphone"));

    endVideoButton = createButton("videoButtonFullScreen", "none");
    endVideoButton->setToolTip(tr("End video call"));

    exitFullScreenButton = createButton("exitFullScreenButton", "none");
    exitFullScreenButton->setToolTip(tr("Exit full screen"));

    connect(videoPreviewButton, &QPushButton::clicked, this,
            &GenericNetCamView::toggleVideoPreview);
    connect(volumeButton, &QPushButton::clicked, this, &GenericNetCamView::volMuteToggle);
    connect(microphoneButton, &QPushButton::clicked, this, &GenericNetCamView::micMuteToggle);
    connect(endVideoButton, &QPushButton::clicked, this, &GenericNetCamView::endVideoCall);
    connect(exitFullScreenButton, &QPushButton::clicked, this,
            &GenericNetCamView::toggleFullScreen);

    buttonPanelLayout->addStretch();
    buttonPanelLayout->addWidget(videoPreviewButton);
    buttonPanelLayout->addWidget(volumeButton);
    buttonPanelLayout->addWidget(microphoneButton);
    buttonPanelLayout->addWidget(endVideoButton);
    buttonPanelLayout->addWidget(exitFullScreenButton);
    buttonPanelLayout->addStretch();
}

QSize GenericNetCamView::getSurfaceMinSize() {
    QSize surfaceSize = videoSurface->minimumSize();
    QSize buttonSize = toggleMessagesButton->size();
    QSize panelSize(0, 45);

    return surfaceSize + buttonSize + panelSize;
}

void GenericNetCamView::setShowMessages(bool show, bool notify) {
    if (!show) {
        toggleMessagesButton->setText(tr("Hide Messages"));
        toggleMessagesButton->setIcon(QIcon());
        return;
    }

    toggleMessagesButton->setText(tr("Show Messages"));

    if (notify) {
        toggleMessagesButton->setIcon(QIcon(Style::getImagePath("chatArea/info.svg")));
    }
}

void GenericNetCamView::toggleFullScreen() {
    if (isFullScreen()) {
        exitFullScreen();
    } else {
        enterFullScreen();
    }
}

void GenericNetCamView::enterFullScreen() {
    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
    showFullScreen();
    enterFullScreenButton->hide();
    toggleMessagesButton->hide();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    const auto screenSize = QGuiApplication::screenAt(this->pos())->geometry();
#else
    const QRect screenSize = QApplication::desktop()->screenGeometry(this);
#endif
    buttonPanel->setGeometry((screenSize.width() / 2) - buttonPanel->width() / 2,
                             screenSize.height() - BTN_PANEL_HEIGHT - 25, BTN_PANEL_WIDTH,
                             BTN_PANEL_HEIGHT);
    buttonPanel->show();
    buttonPanel->activateWindow();
    buttonPanel->raise();
}

void GenericNetCamView::exitFullScreen() {
    setWindowFlags(Qt::Widget);
    showNormal();
    buttonPanel->hide();
    enterFullScreenButton->show();
    toggleMessagesButton->show();
}

void GenericNetCamView::endVideoCall() {
    toggleFullScreen();
    emit videoCallEnd();
}

void GenericNetCamView::toggleVideoPreview() {
    toggleButtonState(videoPreviewButton);
    emit videoPreviewToggle();
}

QPushButton* GenericNetCamView::createButton(const QString& name, const QString& state) {
    QPushButton* btn = new QPushButton();
    btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    btn->setObjectName(name);
    btn->setProperty("state", QVariant(state));
    btn->setStyleSheet(Style::getStylesheet(BTN_STYLE_SHEET_PATH));

    return btn;
}

void GenericNetCamView::updateMuteVolButton(bool isMuted) {
    updateButtonState(volumeButton, !isMuted);
}

void GenericNetCamView::updateMuteMicButton(bool isMuted) {
    updateButtonState(microphoneButton, !isMuted);
}

void GenericNetCamView::toggleButtonState(QPushButton* btn) {
    if (btn->property("state") == BTN_STATE_RED) {
        btn->setProperty("state", BTN_STATE_NONE);
    } else {
        btn->setProperty("state", BTN_STATE_RED);
    }

    btn->setStyleSheet(Style::getStylesheet(BTN_STYLE_SHEET_PATH));
}

void GenericNetCamView::updateButtonState(QPushButton* btn, bool active) {
    if (active) {
        btn->setProperty("state", BTN_STATE_NONE);
    } else {
        btn->setProperty("state", BTN_STATE_RED);
    }

    btn->setStyleSheet(Style::getStylesheet(BTN_STYLE_SHEET_PATH));
}

void GenericNetCamView::keyPressEvent(QKeyEvent* event) {
    int key = event->key();
    if (key == Qt::Key_Escape && isFullScreen()) {
        exitFullScreen();
    }
}

void GenericNetCamView::closeEvent(QCloseEvent* event) {
    exitFullScreen();
    event->ignore();
}
