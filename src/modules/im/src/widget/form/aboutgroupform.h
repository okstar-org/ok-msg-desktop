#ifndef ABOUTGROUPFORM_H
#define ABOUTGROUPFORM_H

#include <QWidget>
#include "src/model/group.h"

namespace Ui {
class AboutGroupForm;
}

class AboutGroupForm : public QWidget {
    Q_OBJECT

public:
    explicit AboutGroupForm(const GroupId& gId, QWidget* parent = nullptr);
    ~AboutGroupForm();
    void init();

    void updateUI();

private:
    Ui::AboutGroupForm* ui;
    GroupId groupId;
    Group* group;

private slots:
    void onSendMessageClicked();
    void doNameChanged(const QString& text);
    void doAliasChanged(const QString& text);
    void doSubjectChanged(const QString& text);
    void doDescChanged(const QString& text);
};

#endif  // ABOUTGROUPFORM_H
