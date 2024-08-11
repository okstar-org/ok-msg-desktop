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

#ifndef NETCAMVIEW_H
#define NETCAMVIEW_H

#include <QVector>
#include "genericnetcamview.h"
#include "src/core/FriendId.h"

struct vpx_image;

class QHBoxLayout;
class VideoSource;
class QFrame;
class MovableWidget;

class NetCamView : public GenericNetCamView {
    Q_OBJECT

public:
    NetCamView(FriendId friendPk, QWidget* parent = nullptr);
    ~NetCamView();

    virtual void show(VideoSource* source, const QString& title);
    virtual void hide();

    void setSource(VideoSource* s);
    void setTitle(const QString& title);
    void toggleVideoPreview();

protected:
    void showEvent(QShowEvent* event) final override;

private slots:
    void updateRatio();

private:
    void updateFrameSize(QSize size);

    VideoSurface* selfVideoSurface;
    MovableWidget* selfFrame;
    FriendId friendPk;
    bool e;
    QVector<QMetaObject::Connection> connections;
};

#endif  // NETCAMVIEW_H
