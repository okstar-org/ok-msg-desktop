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

#include <QAbstractListModel>
#include <QMenu>
#include <QStackedWidget>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QVBoxLayout>
#include <QVector>

#include <memory>

class QIcon;
class QToolButton;
class QButtonGroup;

class EmoticonsWidget : public QMenu {
    Q_OBJECT
public:
    explicit EmoticonsWidget(QWidget* parent = nullptr);

signals:
    void insertEmoticon(QString str);

private slots:
    void onSmileyClicked(const QString& text);
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
    QButtonGroup* pageIndexGroup = nullptr;

public:
    QSize sizeHint() const override;
};

class EmoticonsPageView : public QWidget {
    Q_OBJECT
signals:
    void clicked(int offset);

public:
    EmoticonsPageView(QWidget* parent);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void setRange(int start, int end);

protected:
    void mouseMoveEvent(QMouseEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    int indexAtPostion(const QPoint& pos, bool accurately = false);
    void updateIndex(int index);
    QRect indexRect(int index);
    void drawCell(QPainter* painter, int index);
    int itemSize() const;

private:
    QToolButton* invisible_button = nullptr;
    int start = -1;
    int end = -1;
    int hoverIndex = -1;
    int pressedIndex = -1;
    mutable int _itemSize = -1;
    QStringList displayCache;
};

#endif  // EMOTICONSWIDGET_H
