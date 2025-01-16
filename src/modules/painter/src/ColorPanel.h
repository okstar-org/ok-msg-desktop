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

#include <memory>

#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>

#define TOOLBOX_PANEL_WIDTH 100
#define TOOLBOX_PANEL_HEIGHT 100

namespace module::painter {

class ToolboxPanelColor : public QWidget {
public:
    ToolboxPanelColor(QWidget* parent = nullptr);
    ~ToolboxPanelColor();
    virtual void mousePressEvent(QMouseEvent* event);

private:
    bool _checked;

signals:
    void checked(ToolboxPanelColor*);
};

class ToolboxPanelSize : public QLabel {
public:
    ToolboxPanelSize(QWidget* parent = nullptr);
    ~ToolboxPanelSize();

    virtual void mousePressEvent(QMouseEvent* event);

private:
    void checked(ToolboxPanelSize*);
};

class ToolboxPanelPoint : public QWidget {
public:
    ToolboxPanelPoint(QWidget* parent = nullptr);
    ~ToolboxPanelPoint();

protected:
    virtual void mousePressEvent(QMouseEvent* event);

private:
signals:
    void checked(ToolboxPanelPoint*);
};

class ColorPanel : public QFrame {
private:
    QHBoxLayout* _widthLayout;

public:
    ColorPanel(QWidget* parent);
    ColorPanel(QWidget* parent, int defaultSize, QString defaultColor);
    ~ColorPanel();

    virtual void checkColor(ToolboxPanelColor* color);

    virtual void hide(bool force);
    virtual bool eventFilter(QObject* target, QEvent* event);

private:
    bool _hovered;
    QColor _color;
    void checkSize(ToolboxPanelColor*);
};

}  // namespace module::painter