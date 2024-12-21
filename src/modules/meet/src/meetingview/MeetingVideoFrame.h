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
#include "base/jid.h"
#include "lib/messenger/messenger.h"

class QToolBar;
class QToolButton;
class PopupMenuComboBox;
class QLabel;

namespace module::meet {

class MeetingVideosContainer;
class MeetingParticipant;
class MeetingUser;

class MeetingVideoFrame : public QWidget, public lib::messenger::MessengerMeetHandler {
    Q_OBJECT
public:
    explicit MeetingVideoFrame(const QString& name, QWidget* parent = nullptr);
    ~MeetingVideoFrame() override;
    void reloadTheme();
    void createMeet(const QString& name);
    void retranslateUi();

    void startCounter();
    void stopCounter();

private:
    void creatTopToolBar();
    void creatBottomBar();
    void initConnection();

    void showLayoutPicker();
    void toggleFullScreen();

    void showAudioPopMenu();

    void changeEvent(QEvent* event);

    /**
     * MessengerMeetHandler
     * @param jid
     * @param ready
     * @param props
     */
    void onMeetCreated(const ok::base::Jid& jid,
                       bool ready,
                       const std::map<std::string, std::string>& props) override;

    void onParticipantJoined(const ok::base::Jid& jid,
                             const lib::messenger::Participant& parti) override;

    void onParticipantLeft(const ok::base::Jid& jid, const QString& participant) override;

    void onParticipantVideoFrame(const QString& participant,
                                 const lib::ortc::RendererImage& image) override;

private:
    // for run in UI thread
    void addParticipant(const QString& name, const lib::messenger::Participant& parti);
    void removeParticipant(const QString& name, const QString& participant);

private:
    // 顶部工具
    QToolBar* topToolBar = nullptr;
    QAction* infoAction = nullptr;
    QAction* sharedAction = nullptr;
    QLabel* duraionLabel = nullptr;
    QAction* netInfoAction = nullptr;

    QAction* layoutAction = nullptr;
    QAction* fullScreenAction = nullptr;

    QTimer* callDurationTimer = nullptr;
    QElapsedTimer* timeElapsed;

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

    lib::messenger::MessengerMeet* meet = nullptr;

    // 所有会议人员
    std::mutex prt_mutex;
    QMap<QString, MeetingParticipant*> participantMap;

    // 会议唯一名称
    QString username;

public slots:
    void doLeaveMeet();

private slots:
    void updateDuration();

signals:
    void meetCreated(const QString& name);
    void meetLeft();
    void participantJoined(const QString& name, const lib::messenger::Participant& part);
    void participantLeft(const QString& name, const QString& participant);
};

}  // namespace module::meet
#endif  // !MEETINGVIDEOFRAME_H
