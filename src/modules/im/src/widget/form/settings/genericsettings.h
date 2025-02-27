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

#ifndef GENERICFORM_H
#define GENERICFORM_H

#include <QWidget>
namespace module::im {

class GenericForm : public QWidget {
    Q_OBJECT
public:
    explicit GenericForm(const QPixmap& icon, QWidget* parent = nullptr);
    virtual ~GenericForm() {}

    virtual QString getFormName() = 0;
    QPixmap getFormIcon();

protected:
    bool eventFilter(QObject* o, QEvent* e) final override;
    void eventsInit();
    void showEvent(QShowEvent* e) override;
    void hideEvent(QHideEvent* e) override;

protected:
    QPixmap formIcon;
};
}  // namespace module::im
#endif
