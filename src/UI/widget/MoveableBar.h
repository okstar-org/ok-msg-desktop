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

#include <QObject>
#include <QWidget>

namespace UI {

class MoveableBar : public QWidget {
    Q_OBJECT

public:
    MoveableBar(QWidget* parent = nullptr);
    ~MoveableBar();

    void setMoveable(bool);
    virtual void setTarget(QWidget*);

protected:
    bool _moveable;

    QWidget* m_target;

    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);

private:
    bool m_isPressed = false;
    QPoint m_startMovePos;
};

}  // namespace UI
