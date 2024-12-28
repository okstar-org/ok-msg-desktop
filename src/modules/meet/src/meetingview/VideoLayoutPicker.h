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

#pragma once

#include <QFrame>
#include "MeetingVideoDefines.h"

class QPushButton;
class QButtonGroup;
class QEventLoop;

namespace module::meet {

class VideoLayoutPicker : public QFrame {
    Q_OBJECT
public:
    explicit VideoLayoutPicker(QWidget* parent);
    void setCurrentType(module::meet::VideoLayoutType type);
    module::meet::VideoLayoutType selectedType() const;
    void exec(const QPoint & pos);

private:
    void onTypeChanged(int id, bool checked);

    void closeEvent(QCloseEvent* e);

private:
    QPushButton * appendItem(const QString& text, const QString& svgPath);

private:
    QButtonGroup* typeGroup = nullptr;
    QEventLoop* eventLoop = nullptr;
};
}  // namespace module::meet