#ifndef GROUPCREATEFORM_H
#define GROUPCREATEFORM_H

#include <QWidget>

namespace Ui {
class GroupCreateForm;
}

class GroupCreateForm : public QWidget {
    Q_OBJECT

public:
    explicit GroupCreateForm(QWidget* parent = nullptr);
    ~GroupCreateForm();

signals:
    void confirmed(const QString& groupName);

private:
    Ui::GroupCreateForm* ui;
};

#endif  // GROUPCREATEFORM_H
