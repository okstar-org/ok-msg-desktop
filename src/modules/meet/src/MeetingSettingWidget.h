#ifndef MEETINGSETTINGWIDGET_H
#define MEETINGSETTINGWIDGET_H

#include <QWidget>

namespace Ui {
class MeetingSettingWidget;
}

class MeetingSettingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MeetingSettingWidget(QWidget *parent = nullptr);
    ~MeetingSettingWidget();

private:
    Ui::MeetingSettingWidget *ui;
};

#endif // MEETINGSETTINGWIDGET_H
