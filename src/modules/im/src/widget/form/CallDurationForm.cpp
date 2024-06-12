#include "CallDurationForm.h"
#include "ui_CallDurationForm.h"

#include "base/times.h"
#include <QPushButton>
#include <QTimer>
#include <src/widget/style.h>

CallDurationForm::CallDurationForm(QWidget *parent)
    :QWidget(parent),//
      ui(new Ui::CallDurationForm), //
      callDurationTimer{new QTimer()}, //
      muteOut{false}, muteIn{false} {
  ui->setupUi(this);

  setWindowFlags(Qt::FramelessWindowHint);
  setWindowFlags(Qt::WindowMinMaxButtonsHint);

  ui->videoButton->hide();

  connect(ui->callButton, &QPushButton::clicked, this, &CallDurationForm::onCallEnd);
  connect(ui->micButton, &QPushButton::clicked, this, &CallDurationForm::onMuteOut);
  connect(ui->volButton, &QPushButton::clicked, this, &CallDurationForm::onMuteIn);

  reloadTheme();
  connect(callDurationTimer, &QTimer::timeout, this, &CallDurationForm::onUpdateTime);
  callDurationTimer->start(1000);
  timeElapsed.start();
}

CallDurationForm::~CallDurationForm() {
  QString dhms = base::secondsToDHMS(timeElapsed.elapsed() / 1000);

  //    QString mess = error ? tr("Call with %1 ended unexpectedly. %2")
  //                         : tr("Call with %1 ended. %2");

  //  addSystemInfoMessage(mess.arg(name, dhms), ChatMessage::INFO,
  //                       QDateTime::currentDateTime());
  callDurationTimer->stop();

  delete callDurationTimer;
  callDurationTimer = nullptr;

  delete ui;
}

void CallDurationForm::setContact(const Contact *c) {
  if (!c)
    return;

  ui->avatar->setPixmap(c->getAvatar().scaled(ui->avatar->size()));
  ui->name->setText(c->getDisplayedName());
}

void CallDurationForm::reloadTheme() { setStyleSheet(Style::getStylesheet(QStringLiteral("CallDurationForm/CallDurationForm.css"))); }

void CallDurationForm::closeEvent(QCloseEvent *e) {}

void CallDurationForm::onUpdateTime() {
  auto time = tr("Call duration: ") + base::secondsToDHMS(timeElapsed.elapsed() / 1000);
  ui->duration->setText(time);
}

void CallDurationForm::onCallEnd() { emit endCall(); }

void CallDurationForm::onMuteOut() {
  muteOut = !muteOut;
  ui->micButton->setProperty("state",muteOut?"disabled":"");
  emit muteMicrophone(muteOut);
  reloadTheme();
}

void CallDurationForm::onMuteIn() {
  muteIn = !muteIn;
  ui->volButton->setProperty("state", muteIn?"disabled":"");
  emit muteSpeaker(muteIn);
  reloadTheme();
}
