#include "JoinMeetingWidget.h"
#include "MeetingOptionWidget.h"
#include <base/shadowbackground.h>

#include <QLineEdit>
#include <QVBoxLayout>

JoinMeetingWidget::JoinMeetingWidget(QWidget* parent) : QWidget(parent) {

    setContentsMargins(10, 10, 10, 10);
    ShadowBackground* shadowBack = new ShadowBackground(this);
    shadowBack->setShadowRadius(10);

    idEdit = new QLineEdit(this);
    idEdit->setAlignment(Qt::AlignCenter);
    idEdit->setPlaceholderText(tr("Meeting ID"));
    optionWidget = new MeetingOptionWidget(this);
    optionWidget->setConfirmButtonText(tr("Join Meeting"));

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(idEdit);
    mainLayout->addWidget(optionWidget, 1);
    mainLayout->setAlignment(idEdit, Qt::AlignHCenter);
}
