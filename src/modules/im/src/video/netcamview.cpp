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

#include "netcamview.h"
#include <src/core/coreav.h>
#include <QBoxLayout>
#include <QFrame>
#include <QLabel>
#include "src/core/core.h"
#include "src/friendlist.h"
#include "src/model/friend.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/video/corevideosource.h"
#include "src/video/videosurface.h"
#include "src/widget/tool/movablewidget.h"

NetCamView::NetCamView(FriendId friendPk, QWidget* parent)
        : GenericNetCamView(parent), selfFrame{nullptr}, friendPk{friendPk}, e(false) {
    videoSurface = new VideoSurface(Nexus::getProfile()->loadAvatar(friendPk), this);
    videoSurface->setMinimumHeight(256);

    verLayout->insertWidget(0, videoSurface, 1);

    selfVideoSurface = new VideoSurface(Nexus::getProfile()->loadAvatar(), this, true);
    selfVideoSurface->setObjectName(QStringLiteral("CamVideoSurface"));
    selfVideoSurface->setMouseTracking(true);
    selfVideoSurface->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    selfFrame = new MovableWidget(videoSurface);
    selfFrame->show();

    QHBoxLayout* frameLayout = new QHBoxLayout(selfFrame);
    frameLayout->addWidget(selfVideoSurface);
    frameLayout->setMargin(0);

    updateRatio();
    connections += connect(selfVideoSurface, &VideoSurface::ratioChanged, [this]() {
        selfFrame->setMinimumWidth(selfFrame->minimumHeight() * selfVideoSurface->getRatio());
        QRect boundingRect = selfVideoSurface->getBoundingRect();
        updateFrameSize(boundingRect.size());
        selfFrame->resetBoundary(boundingRect);
    });

    connections += connect(videoSurface, &VideoSurface::boundaryChanged, [this]() {
        QRect boundingRect = videoSurface->getBoundingRect();
        updateFrameSize(boundingRect.size());
        selfFrame->setBoundary(boundingRect);
    });

    connections += connect(videoSurface, &VideoSurface::ratioChanged, [this]() {
        selfFrame->setMinimumWidth(selfFrame->minimumHeight() * videoSurface->getRatio());
        QRect boundingRect = videoSurface->getBoundingRect();
        updateFrameSize(boundingRect.size());
        selfFrame->resetBoundary(boundingRect);
    });

    connections += connect(Nexus::getProfile(), &Profile::selfAvatarChanged,
                           [this](const QPixmap& pixmap) { selfVideoSurface->setAvatar(pixmap); });

    connections += connect(Nexus::getProfile(), &Profile::friendAvatarChanged,
                           [this](FriendId friendPk, const QPixmap& pixmap) {
                               if (this->friendPk == friendPk) videoSurface->setAvatar(pixmap);
                           });

    QRect videoSize = Settings::getInstance().getCamVideoRes();
    qDebug() << "SIZER" << videoSize;
}

NetCamView::~NetCamView() {
    for (QMetaObject::Connection conn : connections) disconnect(conn);
}

void NetCamView::show(VideoSource* source, const QString& title) {
    setSource(source);
    setTitle(title);
    QWidget::show();

    const auto av = CoreAV::getInstance();
    selfVideoSurface->setSource(av->getSelfVideoSource());
}

void NetCamView::hide() {
    setSource(nullptr);
    selfVideoSurface->setSource(nullptr);

    if (selfFrame) selfFrame->deleteLater();

    selfFrame = nullptr;

    QWidget::hide();
}

void NetCamView::setSource(VideoSource* s) { videoSurface->setSource(s); }

void NetCamView::setTitle(const QString& title) { setWindowTitle(title); }

void NetCamView::showEvent(QShowEvent* event) {
    Q_UNUSED(event);
    selfFrame->resetBoundary(videoSurface->getBoundingRect());
}

void NetCamView::updateRatio() {
    selfFrame->setMinimumWidth(selfFrame->minimumHeight() * selfVideoSurface->getRatio());
    selfFrame->setRatio(selfVideoSurface->getRatio());
    selfFrame->update();
}

void NetCamView::updateFrameSize(QSize size) {
    selfFrame->setMaximumSize(size.height() / 3, size.width() / 3);

    if (selfFrame->maximumWidth() > selfFrame->maximumHeight())
        selfFrame->setMaximumWidth(selfFrame->maximumHeight() * selfVideoSurface->getRatio());
    else
        selfFrame->setMaximumHeight(selfFrame->maximumWidth() / selfVideoSurface->getRatio());
}

void NetCamView::toggleVideoPreview() {
    if (selfFrame->isHidden()) {
        selfFrame->show();
    } else {
        selfFrame->hide();
    }
}
