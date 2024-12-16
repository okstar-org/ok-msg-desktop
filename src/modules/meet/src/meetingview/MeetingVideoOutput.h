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

#ifndef MEETINGVIDEOOUTPUT_H
#define MEETINGVIDEOOUTPUT_H

#include <QWidget>

class RoundedPixmapLabel;

namespace module::meet {

class MeetingParticipant;

class MeetingVideoOutput : public QWidget {
    Q_OBJECT
public:
    MeetingVideoOutput(QWidget* parent);

    // 绑定与会者
    void bindParticipant(MeetingParticipant* participant);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* e) override;

private:
    void showVideo();
    void showAvatar();
    // 是否视频输出
    bool hasVideoOutput();
    QRectF calcAvatarRect();

    RoundedPixmapLabel* avatarLabel = nullptr;
    MeetingParticipant* participant = nullptr;
};
}  // namespace module::meet
#endif  // !MEETINGVIDEOOUTPUT_H
