#ifndef MEETINGVIDEOOUTPUT_H
#define MEETINGVIDEOOUTPUT_H

#include <QWidget>

class RoundedPixmapLabel;

class MeetingVideoOutput : public QWidget {
public:
    MeetingVideoOutput(QWidget* parent);
                                                       
private:
    void showVideo();
    void showAvatar();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    RoundedPixmapLabel* avatarLabel = nullptr;
};

#endif  // !MEETINGVIDEOOUTPUT_H
