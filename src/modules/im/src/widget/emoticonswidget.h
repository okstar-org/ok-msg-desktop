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

#ifndef EMOTICONSWIDGET_H
#define EMOTICONSWIDGET_H

#include <QMenu>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QVector>

#include <memory>

class QIcon;

class EmoticonsWidget : public QMenu
{
    Q_OBJECT
public:
    explicit EmoticonsWidget(QWidget* parent = nullptr);

signals:
    void insertEmoticon(QString str);

private slots:
    void onSmileyClicked();
    void onPageButtonClicked();
    void PageButtonsUpdate();

protected:
    void mouseReleaseEvent(QMouseEvent* ev) final override;
    void mousePressEvent(QMouseEvent* ev) final override;
    void wheelEvent(QWheelEvent* event) final override;
    void keyPressEvent(QKeyEvent* e) final override;

private:
    QStackedWidget stack;
    QVBoxLayout layout;
    QList<std::shared_ptr<QIcon>> emoticonsIcons;

public:
    QSize sizeHint() const override;
};

#endif // EMOTICONSWIDGET_H
