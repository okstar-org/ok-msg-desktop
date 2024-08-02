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

#ifndef GENERICNETCAMVIEW_H
#define GENERICNETCAMVIEW_H

#include <QFrame>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#include "src/lib/settings/style.h"
#include "src/video/videosurface.h"

class GenericNetCamView : public QWidget {
    Q_OBJECT
public:
    explicit GenericNetCamView(QWidget* parent);
    QSize getSurfaceMinSize();

signals:
    void showMessageClicked();
    void videoCallEnd();
    void volMuteToggle();
    void micMuteToggle();
    void videoPreviewToggle();

public slots:
    void setShowMessages(bool show, bool notify = false);
    void updateMuteVolButton(bool isMuted);
    void updateMuteMicButton(bool isMuted);

protected:
    QVBoxLayout* verLayout;
    VideoSurface* videoSurface;
    QPushButton* enterFullScreenButton = nullptr;

private:
    QHBoxLayout* buttonLayout = nullptr;
    QPushButton* toggleMessagesButton = nullptr;
    QFrame* buttonPanel = nullptr;
    QPushButton* videoPreviewButton = nullptr;
    QPushButton* volumeButton = nullptr;
    QPushButton* microphoneButton = nullptr;
    QPushButton* endVideoButton = nullptr;
    QPushButton* exitFullScreenButton = nullptr;

private:
    QPushButton* createButton(const QString& name, const QString& state);
    void toggleFullScreen();
    void enterFullScreen();
    void exitFullScreen();
    void endVideoCall();
    void toggleVideoPreview();
    void toggleButtonState(QPushButton* btn);
    void updateButtonState(QPushButton* btn, bool active);
    void keyPressEvent(QKeyEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
};

#endif  // GENERICNETCAMVIEW_H
