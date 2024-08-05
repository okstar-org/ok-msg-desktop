#include "GroupCreateForm.h"
#include "ui_GroupCreateForm.h"

GroupCreateForm::GroupCreateForm(QWidget* parent) : QWidget(parent), ui(new Ui::GroupCreateForm) {
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setWindowTitle(tr("Create new group"));

    connect(ui->confirm, &QPushButton::released, [&]() {
        auto name = ui->group->text();
        if (name.trimmed().isEmpty()) {
            return;
        }
        emit confirmed(name);
    });
}

GroupCreateForm::~GroupCreateForm() {
    disconnect(ui->confirm);
    delete ui;
}
