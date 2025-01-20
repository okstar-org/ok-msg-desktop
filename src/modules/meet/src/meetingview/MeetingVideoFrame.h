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

#include <QMap>
#include <QTime>
#include <QWidget>
#include <mutex>
#include "base/jid.h"
#include "lib/messenger/Messenger.h"

class QToolBar;
class QToolButton;
class QLabel;
class QElapsedTimer;

namespace lib::ui {
class PopupMenuComboBox;
}

namespace module::meet {

class MeetingVideosContainer;
class MeetingParticipant;
class MeetingUser;

class MeetingVideoFrame : public QWidget, public lib::messenger::MessengerMeetHandler {
    Q_OBJECT
public:
    explicit MeetingVideoFrame(const QString& name,
                               lib::ortc::CtrlState ctrlState,
                               QWidget* parent = nullptr);
    ~MeetingVideoFrame() override;
    void reloadTheme();
    void createMeet(const QString& name);
    void retranslateUi();

    void startCounter();
    void stopCounter();

protected:
    /**
     * MessengerMeetHandler
     * @param jid
     * @param ready
     * @param props
     */
    void onMeetCreated(const ok::base::Jid& jid,
                       bool ready,
                       const std::map<std::string, std::string>& props) override;

    void onMeetInitiate(const lib::messenger::IMPeerId& peerId,
                        const lib::ortc::OJingleContentMap& map) override;

    void onParticipantJoined(const ok::base::Jid& jid,
                             const lib::messenger::Participant& parti) override;

    void onParticipantLeft(const ok::base::Jid& jid, const std::string& participant) override;

    void onParticipantVideoFrame(const std::string& participant,
                                 const lib::ortc::RendererImage& image) override;
    void onParticipantMessage(const std::string& participant, const std::string& msg) override;
    void onEnd() override;

private:
    void creatTopToolBar();
    void creatBottomBar();
    void initConnection();

    void showLayoutPicker();
    void toggleFullScreen();

    void showAudioPopMenu();

    void changeEvent(QEvent* event) override;

    // for run in UI thread
    void addParticipant(const QString& name, const lib::messenger::Participant& parti);
    void removeParticipant(const QString& name, const QString& participant);

    void syncAudioVideoState();

    // 顶部工具
    QToolBar* topToolBar = nullptr;
    QAction* infoAction = nullptr;
    QAction* sharedAction = nullptr;
    QLabel* duraionLabel = nullptr;
    QAction* netInfoAction = nullptr;

    QAction* layoutAction = nullptr;
    QAction* fullScreenAction = nullptr;

    QTime duration;
    QTimer* callDurationTimer;
    QElapsedTimer* timeElapsed;

    // 底部按钮区域
    QWidget* bottomBar = nullptr;
    QToolButton* msgButton = nullptr;

    lib::ui::PopupMenuComboBox* audioSettingButton = nullptr;
    lib::ui::PopupMenuComboBox* videoSettingButton = nullptr;
    lib::ui::PopupMenuComboBox* sharedDeskButton = nullptr;
    lib::ui::PopupMenuComboBox* recoardButton = nullptr;
    lib::ui::PopupMenuComboBox* inviteButton = nullptr;

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

    // 控制状态
    lib::ortc::CtrlState ctrlState;

public slots:
    void doLeaveMeet();

private slots:
    void updateDuration();

signals:
    void meetCreated(const QString& name);
    void meetLeft();
    void meetDestroyed();
    void participantJoined(const QString& name, const lib::messenger::Participant& part);
    void participantLeft(const QString& name, const QString& participant);
};

}  // namespace module::meet
#endif  // !MEETINGVIDEOFRAME_H
