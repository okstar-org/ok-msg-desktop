/*
 * Copyright (c) 2022 船山信息 chuanshaninfo.com
 * The project is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PubL v2 for more details.
 */

#ifndef CHATREPLYFORM_H
#define CHATREPLYFORM_H

#include <QWidget>
#include "src/model/MsgId.h"

namespace Ui {
class ChatReplyForm;
}
namespace module::im {

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

    const QString& getText() const {
        return item.content;
    }

private:
    Ui::ChatReplyForm* ui;

    ChatReplyItem item;

signals:
    void removeEvent();
};
}  // namespace module::im
#endif  // CHATREPLYFORM_H
