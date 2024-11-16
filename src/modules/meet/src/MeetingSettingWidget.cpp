#include "MeetingSettingWidget.h"
#include "ui_MeetingSettingWidget.h"
#include <base/shadowbackground.h>

MeetingSettingWidget::MeetingSettingWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MeetingSettingWidget)
{
    setContentsMargins(10, 10, 10, 10);
    ShadowBackground* shadowBack = new ShadowBackground(this);
    shadowBack->setShadowRadius(10);

    ui->setupUi(this);
}

MeetingSettingWidget::~MeetingSettingWidget()
{
    delete ui;
}
