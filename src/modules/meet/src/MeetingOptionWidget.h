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

#pragma once

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

#include <QWidget>
#include <QPointer>

class QToolButton;
class QPushButton;
class RoundedAvatarLabel;
class QHBoxLayout;
class VideoDeviceSettingWidget;
class QSlider;
class QAbstractButton;

class MeetingOptionWidget : public QWidget {
    Q_OBJECT
public:
    MeetingOptionWidget(QWidget* parent = nullptr);
    void setConfirmButtonText(const QString & text);

private:
    RoundedAvatarLabel* avatarLabel = nullptr;

    VideoDeviceSettingWidget* micSpeakSetting = nullptr;
    VideoDeviceSettingWidget* cameraSetting = nullptr;
    VideoDeviceSettingWidget* volumnSetting = nullptr;
    QSlider* volumnSlider = nullptr;
    QPushButton* confirmButton = nullptr;
};

class VideoDeviceSettingWidget : public QWidget
{
    Q_OBJECT
signals:
    void menuRequest();

public:
    VideoDeviceSettingWidget(QWidget* parent = nullptr);
    void setLabel(const QString & text);
    void setWidget(QWidget* widget);
    QAbstractButton* iconButton();

protected:
    void showEvent(QShowEvent* e) override;

private:
    QHBoxLayout* mainLayout = nullptr;
    QToolButton* _iconButton = nullptr;
    QToolButton* menuButton = nullptr;
    QPointer<QWidget> content = nullptr;
};

