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

#include <vector>

#include <QEvent>
#include <QFrame>
#include <QLabel>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QWidget>

#include "Base.h"

namespace module::painter {

class WhiteboardControllerButton : public QLabel {
    Q_OBJECT
public:
    WhiteboardControllerButton(QWidget* parent = nullptr);
    ~WhiteboardControllerButton();

    void setType(WB_CTRL type);
    void setIcon(QString& icon);
    void setActiveIcon(QString& activeIcon);
    void setMoveable(bool moveable);

    void setTarget(QWidget*);

    void setChecked(bool);

    inline void setCheckable(bool checkable) {
        _checkable = checkable;
    }

    WB_CTRL type() {
        return _type;
    }

    bool isChecked() {
        return _isChecked;
    }

    QString icon() {
        return _icon;
    }

protected:
    virtual void showEvent(QShowEvent* event);

    virtual void mousePressEvent(QMouseEvent* event);

    virtual void mouseMoveEvent(QMouseEvent* event);

    virtual void mouseReleaseEvent(QMouseEvent* event);

    virtual void leaveEvent(QEvent* event);

    virtual void enterEvent(QEvent* event);

private:
    WB_CTRL _type;

    QString _icon;
    QString _activeIcon;

    bool _moveable = false;
    bool _checkable = false;

    // 状态
    bool _isChecked = false;
    // 移动
    bool m_isMoved = false;
    bool m_isPressed = false;

    QPoint m_startMovePos;
    QWidget* m_target = nullptr;

signals:
    void clicked();
};

class WhiteboardController : public QWidget {
    Q_OBJECT

public:
    WhiteboardController(QWidget* parent);
    ~WhiteboardController();

protected:
    virtual void ctrl(int i);

private:
    std::vector<WhiteboardControllerButton*> buttons;

signals:
    void checked(WB_CTRL, bool);
};

}  // namespace module::painter
