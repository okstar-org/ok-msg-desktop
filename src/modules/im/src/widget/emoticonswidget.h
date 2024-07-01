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
#include <QAbstractListModel>
#include <QTableView>
#include <QStyledItemDelegate>

#include <memory>

class QIcon;
class QPushButton;

class EmoticonsWidget : public QMenu
{
    Q_OBJECT
public:
    explicit EmoticonsWidget(QWidget* parent = nullptr);

signals:
    void insertEmoticon(QString str);

private slots:
    void onSmileyClicked(const QString & text);
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

class EmoticonsPageModel;

class EmoticonsPageView : public QTableView {
  public:
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    EmoticonsPageView(QWidget *parent);
    void setRange(int start, int end);

  private:
    EmoticonsPageModel *pageModel = nullptr;
};

class EmoticonsPageModel : public QAbstractTableModel {
  public:
    enum PageModelRole {
        EmojiText = Qt::UserRole + 0x0100,
        HasContent
    };
    EmoticonsPageModel(QObject *parent);
    void setRange(int start, int end);
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

  private:
    int start = 0;
    int end = 0;
    int hoverIndex = -1;
    const QList<QStringList> allEmoticons;
};

class EmoticonsPageDelegate : public QStyledItemDelegate
{
  public:
    EmoticonsPageDelegate(QObject *parent);
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
  protected:
    void initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    
  private:
    int iconSize = 24;

};

#endif // EMOTICONSWIDGET_H
