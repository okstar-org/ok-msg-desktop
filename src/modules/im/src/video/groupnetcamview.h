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

#ifndef GROUPNETCAMVIEW_H
#define GROUPNETCAMVIEW_H

#include "genericnetcamview.h"

#include "src/core/FriendId.h"

#include <QMap>

class LabeledVideo;
class QHBoxLayout;

class GroupNetCamView : public GenericNetCamView {
public:
    GroupNetCamView(QString group, QWidget* parent = nullptr);
    void clearPeers();
    void addPeer(const QString& peer, const QString& name);
    void removePeer(const QString& peer);

private slots:
    void onUpdateActivePeer();
    void friendAvatarChanged(QString friendPk, const QPixmap& pixmap);

private:
    struct PeerVideo {
        LabeledVideo* video;
    };

    void setActive(const FriendId& peer = FriendId{});

    QHBoxLayout* horLayout;
    QMap<QString, PeerVideo> videoList;
    LabeledVideo* videoLabelSurface;
    LabeledVideo* selfVideoSurface;
    int activePeer;
    QString group;
};

#endif  // GROUPNETCAMVIEW_H
