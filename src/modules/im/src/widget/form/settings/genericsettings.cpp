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

#include "genericsettings.h"

#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QSpinBox>

/**
 * @class GenericForm
 *
 * This is abstract class used as superclass for all settings forms.
 * It provides correct behaviour of controls for settings forms.
 */

GenericForm::GenericForm(const QPixmap& icon, QWidget* parent) : QWidget(parent), formIcon(icon) {}

QPixmap GenericForm::getFormIcon() { return formIcon; }

/**
 * @brief Prevent stealing mouse wheel scroll.
 *
 * Scrolling event won't be transmitted to comboboxes or spinboxes.
 * You can scroll through general settings without accidentially changing
 * theme / skin / icons etc.
 * @see GenericForm::eventFilter(QObject *o, QEvent *e) at the bottom of this file for more
 */
void GenericForm::eventsInit() {
    for (QComboBox* cb : findChildren<QComboBox*>()) {
        cb->installEventFilter(this);
        cb->setFocusPolicy(Qt::StrongFocus);
    }

    for (QSpinBox* sp : findChildren<QSpinBox*>()) {
        sp->installEventFilter(this);
        sp->setFocusPolicy(Qt::WheelFocus);
    }

    for (QCheckBox* cb :
         findChildren<QCheckBox*>())  // this one is to allow scrolling on checkboxes
        cb->installEventFilter(this);
}

/**
 * @brief Ignore scroll on different controls.
 * @param o Object which has been installed for the watched object.
 * @param e Event object.
 * @return True to stop it being handled further, false otherwise.
 */
bool GenericForm::eventFilter(QObject* o, QEvent* e) {
    if ((e->type() == QEvent::Wheel) &&
        (qobject_cast<QComboBox*>(o) || qobject_cast<QAbstractSpinBox*>(o) ||
         qobject_cast<QCheckBox*>(o))) {
        e->ignore();
        return true;
    }

    return QWidget::eventFilter(o, e);
}
