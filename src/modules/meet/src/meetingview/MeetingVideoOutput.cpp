#include "MeetingVideoOutput.h"
#include "base/RoundedPixmapLabel.h"

MeetingVideoOutput::MeetingVideoOutput(QWidget* parent) : QWidget(parent)
{
    
}

void MeetingVideoOutput::showVideo() { 
    if (avatarLabel)
    {
        avatarLabel->deleteLater();
        avatarLabel = nullptr;
    }
}

void MeetingVideoOutput::showAvatar() {

    if (!avatarLabel)
    {
        avatarLabel = new RoundedPixmapLabel(this);
        avatarLabel->setContentsSize(QSize(120, 120));
    }
    avatarLabel->setGeometry(this->rect());
    avatarLabel->raise();
}

void MeetingVideoOutput::resizeEvent(QResizeEvent* event)
{
    if (avatarLabel)
    {
        avatarLabel->setGeometry(this->rect());
        avatarLabel->raise();
    }
}
