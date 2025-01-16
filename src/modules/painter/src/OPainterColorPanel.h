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
#ifndef OPAINTERCOLORPANEL_H
#define OPAINTERCOLORPANEL_H

#include <QToolButton>
#include <QWidget>

namespace Ui {
class OPainterColorPanel;
}

namespace module::painter {

class OPainterColorPanel : public QWidget {
    Q_OBJECT

public:
    explicit OPainterColorPanel(QWidget* parent = nullptr);
    ~OPainterColorPanel();

    void init();

    void hide(bool force);

private:
    Ui::OPainterColorPanel* ui;

    bool _hovered = false;

    void checkWeight(QToolButton* weight);
    void checkColor(QToolButton* color);

signals:
    void colorChange(QColor color);
    void weightChange(int weight);

protected:
    bool eventFilter(QObject* target, QEvent* event);

protected slots:
    virtual void on_color_1_clicked(bool isChecked);
    virtual void on_color_2_clicked(bool isChecked);
    virtual void on_color_3_clicked(bool isChecked);
    virtual void on_color_4_clicked(bool isChecked);
    virtual void on_color_5_clicked(bool isChecked);
    virtual void on_color_6_clicked(bool isChecked);
    virtual void on_color_7_clicked(bool isChecked);
    virtual void on_color_8_clicked(bool isChecked);
    virtual void on_color_9_clicked(bool isChecked);
    virtual void on_color_10_clicked(bool isChecked);
    virtual void on_color_11_clicked(bool isChecked);
    virtual void on_color_12_clicked(bool isChecked);

    virtual void on_weight_1_clicked(bool isChecked);
    virtual void on_weight_2_clicked(bool isChecked);
    virtual void on_weight_3_clicked(bool isChecked);
    virtual void on_weight_4_clicked(bool isChecked);
};

}  // namespace module::painter
#endif  // OPAINTERCOLORPANEL_H
