#include "ChatReplyForm.h"
#include "ui_ChatReplyForm.h"

ChatReplyForm::ChatReplyForm(const ChatReplyItem& item, QWidget* parent)
        : QWidget(parent), ui(new Ui::ChatReplyForm), item{item} {
    ui->setupUi(this);

    ui->nickname->setText(item.nickname);
    ui->content->setText(item.content);
    ui->removeReplyButton->setCursor(Qt::PointingHandCursor);

    connect(ui->removeReplyButton, &QPushButton::clicked, [&](bool c) { emit removeEvent(); });
}

ChatReplyForm::~ChatReplyForm() { delete ui; }
