#ifndef MEETINGVIDEOOUTPUT_H
#define MEETINGVIDEOOUTPUT_H

#include <QWidget>

class RoundedPixmapLabel;
class MeetingParticipant;

class MeetingVideoOutput : public QWidget {
public:
    MeetingVideoOutput(QWidget* parent);
                                                       
    // 绑定与会者
    void bindParticipant(MeetingParticipant* participant);
                                                       
private:
    void showVideo();
    void showAvatar();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    RoundedPixmapLabel* avatarLabel = nullptr;
};

#endif  // !MEETINGVIDEOOUTPUT_H
