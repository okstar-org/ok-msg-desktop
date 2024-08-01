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

#include "groupnetcamview.h"
#include <QBoxLayout>
#include <QMap>
#include <QScrollArea>
#include <QSplitter>
#include <QTimer>
#include "src/audio/audio.h"
#include "src/core/FriendId.h"
#include "src/core/core.h"
#include "src/friendlist.h"
#include "src/model/friend.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/video/videosurface.h"
#include "src/widget/tool/croppinglabel.h"

#include <QDebug>
class LabeledVideo : public QFrame {
public:
    LabeledVideo(const QPixmap& avatar, QString fontColorString, QWidget* parent = nullptr,
                 bool expanding = true)
            : QFrame(parent) {
        qDebug() << "Created expanding? " << expanding;
        videoSurface = new VideoSurface(avatar, nullptr, expanding);
        videoSurface->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        videoSurface->setMinimumHeight(32);

        connect(videoSurface, &VideoSurface::ratioChanged, this, &LabeledVideo::updateSize);
        label = new CroppingLabel(this);
        label->setTextFormat(Qt::PlainText);
        label->setStyleSheet(QString("color: %1").arg(fontColorString));

        label->setAlignment(Qt::AlignCenter);

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(videoSurface, 1);
        layout->addWidget(label);
    }

    ~LabeledVideo() {}

    VideoSurface* getVideoSurface() const { return videoSurface; }

    void setText(const QString& text) { label->setText(text); }

    QString getText() const { return label->text(); }

    void setActive(bool active = true) {
        if (active)
            setStyleSheet("QFrame { background-color: #414141; border-radius: 10px; }");
        else
            setStyleSheet(QString());
    }

protected:
    void resizeEvent(QResizeEvent* event) final override {
        updateSize();
        QWidget::resizeEvent(event);
    }

private slots:
    void updateSize() {
        if (videoSurface->isExpanding()) {
            int width = videoSurface->height() * videoSurface->getRatio();
            videoSurface->setMinimumWidth(width);
            videoSurface->setMaximumWidth(width);
        }
    }

private:
    CroppingLabel* label;
    VideoSurface* videoSurface;
};

GroupNetCamView::GroupNetCamView(QString group, QWidget* parent)
        : GenericNetCamView(parent), group(group) {
    videoLabelSurface = new LabeledVideo(QPixmap(), "white", this, false);
    videoSurface = videoLabelSurface->getVideoSurface();
    videoSurface->setMinimumHeight(256);
    videoSurface->setContentsMargins(6, 6, 6, 0);
    videoLabelSurface->setContentsMargins(0, 0, 0, 0);
    videoLabelSurface->layout()->setMargin(0);
    videoLabelSurface->setStyleSheet("QFrame { background-color: black; }");

    // remove full screen button in audio group chat since it's useless there
    enterFullScreenButton->hide();

    QSplitter* splitter = new QSplitter(Qt::Vertical, this);
    splitter->setChildrenCollapsible(false);
    verLayout->insertWidget(0, splitter, 1);
    splitter->addWidget(videoLabelSurface);
    splitter->setStyleSheet(
            "QSplitter { background-color: black; } QSplitter::handle { background-color: black; "
            "}");

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Note this is needed to prevent oscillations that result in segfaults
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    scrollArea->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));

    scrollArea->setFrameStyle(QFrame::NoFrame);
    QWidget* widget = new QWidget(nullptr);
    scrollArea->setWidgetResizable(true);
    horLayout = new QHBoxLayout(widget);
    horLayout->addStretch(1);

    selfVideoSurface = new LabeledVideo(Nexus::getProfile()->loadAvatar(), "black", this);
    horLayout->addWidget(selfVideoSurface);

    horLayout->addStretch(1);
    splitter->addWidget(scrollArea);
    scrollArea->setWidget(widget);

    QTimer* timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, this, &GroupNetCamView::onUpdateActivePeer);
    timer->start();

    connect(Nexus::getProfile(), &Profile::selfAvatarChanged, [this](const QPixmap& pixmap) {
        selfVideoSurface->getVideoSurface()->setAvatar(pixmap);
        setActive();
    });
    connect(Core::getInstance(), &Core::usernameSet, [this](const QString& username) {
        selfVideoSurface->setText(username);
        setActive();
    });

    //    connect(Nexus::getProfile(), &Profile::friendAvatarChanged, this,
    //            &GroupNetCamView::friendAvatarChanged);

    selfVideoSurface->setText(Core::getInstance()->getUsername());
}

void GroupNetCamView::clearPeers() {
    for (const auto& peerPk : videoList.keys()) {
        removePeer(peerPk);
    }
}

void GroupNetCamView::addPeer(const QString& peer, const QString& name) {
    qDebug() << __func__;
    //    QPixmap groupAvatar = Nexus::getProfile()->loadAvatar(peer);
    //    LabeledVideo* labeledVideo = new LabeledVideo(groupAvatar, "black", this);
    //    labeledVideo->setText(name);
    //    horLayout->insertWidget(horLayout->count() - 1, labeledVideo);
    //    PeerVideo peerVideo;
    //    peerVideo.video = labeledVideo;
    //    videoList.insert(peer, peerVideo);

    //    setActive();
}

void GroupNetCamView::removePeer(const QString& peer) {
    auto peerVideo = videoList.find(peer);

    if (peerVideo != videoList.end()) {
        LabeledVideo* labeledVideo = peerVideo.value().video;
        horLayout->removeWidget(labeledVideo);
        labeledVideo->deleteLater();
        videoList.remove(peer);

        setActive();
    }
}

void GroupNetCamView::onUpdateActivePeer() { setActive(); }

void GroupNetCamView::setActive(const FriendId& peer) {
    if (!peer.isValid()) {
        videoLabelSurface->setText(selfVideoSurface->getText());
        activePeer = -1;
        return;
    }

    // TODO(sudden6): check if we can remove the code, it won't be reached right now
#if 0
    auto peerVideo = videoList.find(peer);

    if (peerVideo != videoList.end()) {
        // When group video exists:
        // videoSurface->setSource(peerVideo.value()->getVideoSurface()->source);

        auto lastVideo = videoList.find(activePeer);

        if (lastVideo != videoList.end())
            lastVideo.value().video->setActive(false);

        LabeledVideo* labeledVideo = peerVideo.value().video;
        videoLabelSurface->setText(labeledVideo->getText());
        videoLabelSurface->getVideoSurface()->setAvatar(labeledVideo->getVideoSurface()->getAvatar());
        labeledVideo->setActive();

        activePeer = peer;
    }
#endif
}

void GroupNetCamView::friendAvatarChanged(QString friendPk, const QPixmap& pixmap) {
    auto peerVideo = videoList.find(friendPk);
    if (peerVideo != videoList.end()) {
        peerVideo.value().video->getVideoSurface()->setAvatar(pixmap);
        setActive();
    }
}
