#ifndef CHATREPLYFORM_H
#define CHATREPLYFORM_H

#include <QWidget>
#include "src/model/MsgId.h"

namespace Ui {
class ChatReplyForm;
}

struct ChatReplyItem {
    MsgId id;
    QString nickname;
    QString content;
};

class ChatReplyForm : public QWidget {
    Q_OBJECT

public:
    explicit ChatReplyForm(const ChatReplyItem& item, QWidget* parent = nullptr);
    ~ChatReplyForm();

private:
    Ui::ChatReplyForm* ui;

    ChatReplyItem item;

signals:
    void removeEvent();
};

#endif  // CHATREPLYFORM_H
