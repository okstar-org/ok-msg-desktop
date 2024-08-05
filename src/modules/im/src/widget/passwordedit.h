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

#ifndef PASSWORDEDIT_H
#define PASSWORDEDIT_H

#include <QAction>
#include <QLineEdit>

class PasswordEdit : public QLineEdit {
    Q_OBJECT
public:
    explicit PasswordEdit(QWidget* parent);
    ~PasswordEdit();

protected:
    virtual void showEvent(QShowEvent* event);
    virtual void hideEvent(QHideEvent* event);

private:
    class EventHandler : QObject {
    public:
        QVector<QAction*> actions;

        EventHandler();
        ~EventHandler();
        void updateActions();
        bool eventFilter(QObject* obj, QEvent* event);
    };

    void registerHandler();
    void unregisterHandler();

private:
    QAction* action;

    static EventHandler* eventHandler;
};
#endif  // PASSWORDEDIT_H
