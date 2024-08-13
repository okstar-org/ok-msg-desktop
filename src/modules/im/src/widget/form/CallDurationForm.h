#ifndef CALLDURATTIONFORM_H
#define CALLDURATTIONFORM_H

#include <QElapsedTimer>
#include <QLabel>
#include <QWidget>

#include <src/model/contact.h>
#include "src/video/genericnetcamview.h"

namespace Ui {
class CallDurationForm;
}

class CallDurationForm : public QWidget {
    Q_OBJECT

public:
    explicit CallDurationForm(QWidget* parent = nullptr);
    ~CallDurationForm();
    void setContact(const Contact* c);
    void reloadTheme();

    GenericNetCamView* createNetcam();
    void showNetcam();
    void hideNetcam();
    void showAvatar();
signals:
    void endCall();
    void muteMicrophone(bool);
    void muteSpeaker(bool);

protected:
    void closeEvent(QCloseEvent* e) override;

private:
    Ui::CallDurationForm* ui;
    const Contact* contact;
    QTimer* callDurationTimer;
    QElapsedTimer timeElapsed;

    bool muteOut;
    bool muteIn;
    GenericNetCamView* netcam;

private slots:
    void onUpdateTime();
    void onCallEnd();
    // 禁止麦克风
    void onMuteOut();
    // 禁止扬声器
    void onMuteIn();
};

#endif  // CALLDURATTIONFORM_H
