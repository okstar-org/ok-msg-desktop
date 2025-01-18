//
// Created by gaojie on 25-1-17.
//

#include "StartRoomWidget.h"
#include "Widget.h"

#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include "MeetingOptionWidget.h"
#include "base/shadowbackground.h"

namespace module::classroom {

static QPushButton* createButton(const QString& text, QWidget* parent, const QString& id) {
    QPushButton* button = new QPushButton(text, parent);
    button->setObjectName(id);
    button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    button->setCursor(Qt::PointingHandCursor);
    return button;
}

StartRoomWidget::StartRoomWidget(QWidget* parent) : OWidget(parent) {
    widget = dynamic_cast<Widget*>(parent);

    setContentsMargins(10, 10, 10, 10);

    auto shadowBack = new ok::base::ShadowBackground(this);
    shadowBack->setShadowRadius(10);

    meetingNameEdit = new QLineEdit(this);
    meetingNameEdit->setText("test");
    meetingNameEdit->setAlignment(Qt::AlignCenter);
    meetingNameEdit->setPlaceholderText(tr("Meeting Name"));
    optionWidget = new MeetingOptionWidget(this);

    confirmButton = createButton(tr("Start Meeting"), optionWidget, "confirm");
    connect(confirmButton, &QPushButton::clicked, [this]() {
        auto n = getName();
        if (n.isEmpty()) {
            return;
        }
        emit requstStartMeeting(n, optionWidget->getCtrlState());
    });

    shareButton = createButton(tr("Share"), optionWidget, "share");
    connect(shareButton, &QPushButton::clicked, this, &StartRoomWidget::requstShareMeeting);
    disbandButton = createButton(tr("Disband"), optionWidget, "disband");
    connect(disbandButton, &QPushButton::clicked, this, &StartRoomWidget::requstDisbandMeeting);

    optionWidget->addFooterButton(disbandButton);
    optionWidget->addFooterButton(shareButton);
    optionWidget->addFooterButton(confirmButton);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(meetingNameEdit);
    mainLayout->addWidget(optionWidget, 1);
    mainLayout->setAlignment(meetingNameEdit, Qt::AlignHCenter);

    connect(meetingNameEdit, &QLineEdit::returnPressed, this,
            [this]() { emit confirmButton->clicked(); });

    updateUi();
}

StartRoomWidget::~StartRoomWidget() {}
void StartRoomWidget::setMeetingState(RoomState state) {}
void StartRoomWidget::focusInput() {}

QString StartRoomWidget::getName() {
    return meetingNameEdit->text();
}
void StartRoomWidget::updateUi() {
    auto meetingState = widget->getState();
    switch (meetingState) {
        case RoomState::None:
            shareButton->setVisible(false);
            disbandButton->setVisible(false);
            confirmButton->setVisible(true);
            confirmButton->setEnabled(true);
            meetingNameEdit->setReadOnly(false);
            break;
        case RoomState::Creating:
            shareButton->setVisible(false);
            disbandButton->setVisible(false);
            confirmButton->setVisible(true);
            confirmButton->setEnabled(false);
            meetingNameEdit->setReadOnly(true);
            break;
        case RoomState::Meeting:
            shareButton->setVisible(true);
            disbandButton->setVisible(true);
            confirmButton->setVisible(false);
            meetingNameEdit->setReadOnly(true);
            break;
        default:
            break;
    }
}

}  // namespace module::classroom
