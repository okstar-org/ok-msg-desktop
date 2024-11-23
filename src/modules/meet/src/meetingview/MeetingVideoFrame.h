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

#ifndef MEETINGVIDEOFRAME_H
#define MEETINGVIDEOFRAME_H

#include <QWidget>
#include "MeetingVideoDefines.h"


class QToolBar;
class QToolButton;
class PopupMenuComboBox;
class QLabel;

namespace lib::messenger{
class IMConference;
}



class MeetingVideosContainer;

class MeetingVideoFrame : public QWidget
{
    Q_OBJECT
public:
    MeetingVideoFrame(QWidget* parent = nullptr);
    void reloadTheme();

    void createConference(const QString& name);

private:
    void creatTopToolBar();
    void creatBottomBar();
    void initConnection();

    void showLayoutPicker();
    void toggleFullScreen();
    void updateDuration();
    void showAudioPopMenu();

    void changeEvent(QEvent* event);
public:
    void retranslateUi();
private:

    // 顶部工具
    QToolBar* topToolBar = nullptr;
    QAction * infoAction = nullptr;
    QAction * sharedAction = nullptr;
    QLabel* duraionLabel = nullptr;
    QAction* netInfoAction = nullptr;
    
    QAction* layoutAction = nullptr;
    QAction* fullScreenAction = nullptr;

    // 底部按钮区域
    QWidget* bottomBar = nullptr;
    QToolButton* msgButton = nullptr;

    PopupMenuComboBox* audioSettingButton = nullptr;
    PopupMenuComboBox* videoSettingButton = nullptr;
    PopupMenuComboBox* sharedDeskButton = nullptr;
    PopupMenuComboBox* recoardButton = nullptr;
    PopupMenuComboBox* inviteButton = nullptr;

    QToolButton* leaveButton = nullptr;

    QToolButton* securityButton = nullptr;
    QToolButton* moreOptionButon = nullptr;

    // 会议视频布局区域
    MeetingVideosContainer* videosLayout = nullptr;

    lib::messenger::IMConference* conference;
};


#endif  // !MEETINGVIDEOFRAME_H
