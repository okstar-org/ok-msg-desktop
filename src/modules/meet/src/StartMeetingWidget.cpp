#include "StartMeetingWidget.h"
#include "MeetingOptionWidget.h"
#include <base/shadowbackground.h>

#include <QLineEdit>
#include <QVBoxLayout>

StartMeetingWidget::StartMeetingWidget(QWidget* parent) :QWidget(parent){

    setContentsMargins(10, 10, 10, 10);
    ShadowBackground* shadowBack = new ShadowBackground(this);
    shadowBack->setShadowRadius(10);

    meetingNameEdit = new QLineEdit(this);
    meetingNameEdit->setAlignment(Qt::AlignCenter);
    meetingNameEdit->setPlaceholderText(tr("Meeting Name"));
    optionWidget = new MeetingOptionWidget(this);
    optionWidget->setConfirmButtonText(tr("Start Meeting"));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(meetingNameEdit);
    mainLayout->addWidget(optionWidget, 1);
    mainLayout->setAlignment(meetingNameEdit, Qt::AlignHCenter);

    connect(optionWidget, &MeetingOptionWidget::confirmed, this,
            &StartMeetingWidget::requstStartMeeting);
}

