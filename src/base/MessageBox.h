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

//
// Created by gaojie on 24-10-1.
//

#pragma once

#include <QMessageBox>
#include <QWidget>

namespace ok::base {

class MessageBox {
public:
    static inline void information(QWidget* parent, const QString& title, const QString& text,
                                   QMessageBox::StandardButton button = QMessageBox::Ok) {
        QMessageBox::information(parent, title, text, button);
    }

    static inline void warning(QWidget* parent, const QString& title, const QString& text,
                               QMessageBox::StandardButton button = QMessageBox::Ok) {
        QMessageBox::warning(parent, title, text, button);
    }

    static inline void critical(QWidget* parent, const QString& title, const QString& text,
                                QMessageBox::StandardButton button = QMessageBox::Ok) {
        QMessageBox::critical(parent, title, text, button);
    }
};

}  // namespace ok::base
