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

#include <QFrame>
#include <QPointer>

class QPushButton;
class RoundedPixmapLabel;
class PopupMenuComboBox;
class QSlider;

class MeetingOptionWidget : public QWidget {
    Q_OBJECT

signals:
    void confirmed();

public:
    MeetingOptionWidget(QWidget* parent = nullptr);
    void setConfirmButtonText(const QString & text);

protected:
    void showEvent(QShowEvent* event) override;

private:
    RoundedPixmapLabel* avatarLabel = nullptr;

    PopupMenuComboBox* micSpeakSetting = nullptr;
    PopupMenuComboBox* cameraSetting = nullptr;
    PopupMenuComboBox* volumnSetting = nullptr;
    QSlider* volumnSlider = nullptr;
    QPushButton* confirmButton = nullptr;
};
