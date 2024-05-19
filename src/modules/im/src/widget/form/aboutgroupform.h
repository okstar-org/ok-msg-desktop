#ifndef ABOUTGROUPFORM_H
#define ABOUTGROUPFORM_H

#include <QWidget>
#include "src/model/group.h"

namespace Ui {
class AboutGroupForm;
}

class AboutGroupForm : public QWidget
{
    Q_OBJECT

public:
    explicit AboutGroupForm(const GroupId & gId, QWidget *parent = nullptr);
    ~AboutGroupForm();
    void init();

    void updateUI();

private:

    Ui::AboutGroupForm *ui;
    GroupId groupId;
     Group* group;

private slots:
    void onSendMessageClicked();
};

#endif // ABOUTGROUPFORM_H
