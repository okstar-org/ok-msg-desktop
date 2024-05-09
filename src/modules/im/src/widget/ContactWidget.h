#ifndef CONTACTWIDGET_H
#define CONTACTWIDGET_H

#include <QWidget>

namespace Ui {
class ContactWidget;
}

class ContactWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ContactWidget(QWidget *parent = nullptr);
    ~ContactWidget();

private:
    Ui::ContactWidget *ui;
};

#endif // CONTACTWIDGET_H
