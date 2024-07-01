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

#include "emoticonswidget.h"
#include "src/persistence/settings.h"
#include "src/persistence/smileypack.h"
#include "src/widget/style.h"

#include <QFile>
#include <QGridLayout>
#include <QLayout>
#include <QMouseEvent>
#include <QPushButton>
#include <QRadioButton>
#include <QHeaderView>
#include <QApplication>

#include <math.h>

constexpr int emoji_layout_rows = 8;
constexpr int emoji_layout_cols = 8;
constexpr int emoji_cell_ext = 6;

EmoticonsWidget::EmoticonsWidget(QWidget* parent)
    : QMenu(parent)
{
    setStyleSheet(Style::getStylesheet("emoticonWidget/emoticonWidget.css"));
    setLayout(&layout);
    layout.addWidget(&stack);

    QWidget* pageButtonsContainer = new QWidget;
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    pageButtonsContainer->setLayout(buttonLayout);

    layout.addWidget(pageButtonsContainer);

    const int maxCols = 8;
    const int maxRows = 8;
    const int itemsPerPage = emoji_layout_rows * emoji_layout_cols;

    const QList<QStringList>& emoticons = SmileyPack::getInstance().getEmoticons();
    int itemCount = emoticons.size();
    int pageCount = ceil(float(itemCount) / float(itemsPerPage));
    int currPage = 0;
    int currItem = 0;
    int row = 0;
    int col = 0;

    // respect configured emoticon size
    const int px = Settings::getInstance().getEmojiFontPointSize();
    const QSize size(px, px);

    // create pages
    buttonLayout->addStretch();
    for (int i = 0; i < pageCount; ++i) {
        EmoticonsPageView *page = new EmoticonsPageView(this);
        page->setRange(itemsPerPage * i, std::min(itemsPerPage * (i + 1), itemCount) - 1);
        stack.addWidget(page);
        connect(page, &EmoticonsPageView::clicked, this, [this](const QModelIndex &index) {
            if (index.isValid())
            {
                QVariant val = index.data(EmoticonsPageModel::EmojiText);
                if (val.isValid())
                    onSmileyClicked(val.toString());
            }
        });

        // page buttons are only needed if there is more than 1 page
        if (pageCount > 1) {
            QRadioButton* pageButton = new QRadioButton;
            pageButton->setProperty("pageIndex", i);
            pageButton->setCursor(Qt::PointingHandCursor);
            pageButton->setChecked(i == 0);
            buttonLayout->addWidget(pageButton);

            connect(pageButton, &QRadioButton::clicked, this, &EmoticonsWidget::onPageButtonClicked);
        }
    }
    buttonLayout->addStretch();

    // calculates sizeHint
    layout.activate();
}

void EmoticonsWidget::onSmileyClicked(const QString &text) {
    // emit insert emoticon
    QString sequence = text;
    sequence.replace("&lt;", "<").replace("&gt;", ">");
    emit insertEmoticon(sequence);
}

void EmoticonsWidget::onPageButtonClicked()
{
    QWidget* sender = qobject_cast<QRadioButton*>(QObject::sender());
    if (sender) {
        int page = sender->property("pageIndex").toInt();
        stack.setCurrentIndex(page);
    }
}

QSize EmoticonsWidget::sizeHint() const
{
    return layout.sizeHint();
}

void EmoticonsWidget::mouseReleaseEvent(QMouseEvent* ev)
{
    if (!rect().contains(ev->pos()))
        hide();
}

void EmoticonsWidget::mousePressEvent(QMouseEvent*)
{
}

void EmoticonsWidget::wheelEvent(QWheelEvent* e)
{
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    const bool vertical = qAbs(e->angleDelta().y()) >= qAbs(e->angleDelta().x());
    const int delta = vertical ? e->angleDelta().y() : e->angleDelta().x();

    if (vertical) {
        if (delta < 0) {
#else
    if (e->orientation() == Qt::Vertical) {
        if (e->delta() < 0) {
#endif
            stack.setCurrentIndex(stack.currentIndex() + 1);
        } else {
            stack.setCurrentIndex(stack.currentIndex() - 1);
        }
        emit PageButtonsUpdate();
    }
}

void EmoticonsWidget::PageButtonsUpdate()
{
    QList<QRadioButton*> pageButtons = this->findChildren<QRadioButton*>(QString());
    foreach (QRadioButton* t_pageButton, pageButtons) {
        if (t_pageButton->property("pageIndex").toInt() == stack.currentIndex())
            t_pageButton->setChecked(true);
        else
            t_pageButton->setChecked(false);
    }
}

void EmoticonsWidget::keyPressEvent(QKeyEvent* e)
{
    Q_UNUSED(e)
    hide();
}



EmoticonsPageView::EmoticonsPageView(QWidget *parent) : QTableView(parent) {
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    horizontalHeader()->setVisible(false);
    verticalHeader()->setVisible(false);
    horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    int iconSize = Settings::getInstance().getEmojiFontPointSize();
    horizontalHeader()->setDefaultSectionSize(iconSize + emoji_cell_ext);
    verticalHeader()->setDefaultSectionSize(iconSize + emoji_cell_ext);
    
    setShowGrid(false);
    setFrameShape(QFrame::NoFrame);
    pageModel = new EmoticonsPageModel(this);
    setModel(pageModel);
    setItemDelegate(new EmoticonsPageDelegate(this));
}

QSize EmoticonsPageView::sizeHint() const
{
    return minimumSizeHint();
}

QSize EmoticonsPageView::minimumSizeHint() const {

    int w = horizontalHeader()->length();
    int h = verticalHeader()->length();
    return QSize(w, h);
}

void EmoticonsPageView::setRange(int start, int end)
{
    pageModel->setRange(start, end);
    updateGeometry();
}


EmoticonsPageModel::EmoticonsPageModel(QObject *parent) 
    : QAbstractTableModel(parent), allEmoticons(SmileyPack::getInstance().getEmoticons()) {

}

void EmoticonsPageModel::setRange(int start, int end)
{
    this->start = std::max(start, 0);
    this->end = std::max(end, 0);
    if (this->end < this->start)
        std::swap(this->start, this->end);
}

int EmoticonsPageModel::rowCount(const QModelIndex &parent) const {
    return emoji_layout_rows;
}

int EmoticonsPageModel::columnCount(const QModelIndex &parent) const {
    return emoji_layout_cols;
}

QVariant EmoticonsPageModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return QVariant();
    int offset = index.row() * emoji_layout_cols + index.column() + start;
    if (offset >= allEmoticons.size())
        return QVariant();

    const QStringList & set = allEmoticons.at(offset);
    switch (role) {
    case Qt::DisplayRole:
        return QVariant();
    case Qt::DecorationRole:
        return *SmileyPack::getInstance().getAsIcon(set.value(0)).get();
    case Qt::ToolTipRole:
        return set.join(' ');
    case EmojiText:
        return set.value(0);
    case HasContent:
        return true;
    default:
        break;
    }
    return QVariant();
}

Qt::ItemFlags EmoticonsPageModel::flags(const QModelIndex &index) const
{
    if (data(index, HasContent).toBool())
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    return Qt::ItemFlags();
}

EmoticonsPageDelegate::EmoticonsPageDelegate(QObject *parent):QStyledItemDelegate(parent)
{
    iconSize = Settings::getInstance().getEmojiFontPointSize();
}

QSize EmoticonsPageDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QSize(iconSize, iconSize) + QSize(emoji_cell_ext, emoji_cell_ext);
}

void EmoticonsPageDelegate::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    option->index = index;
    option->features = QStyleOptionViewItem::HasDisplay;
    option->decorationSize = QSize(iconSize, iconSize); 
    QVariant value = index.data(Qt::DecorationRole);
    if (value.isValid() && !value.isNull()) {
        switch (value.userType()) {
        case QMetaType::QIcon: {
            option->icon = qvariant_cast<QIcon>(value);
            break;
        }
        case QMetaType::QColor: {
            QPixmap pixmap(option->decorationSize);
            pixmap.fill(qvariant_cast<QColor>(value));
            option->icon = QIcon(pixmap);
            break;
        }
        case QMetaType::QImage: {
            QImage image = qvariant_cast<QImage>(value);
            option->icon = QIcon(QPixmap::fromImage(image));
            option->decorationSize = image.size() / image.devicePixelRatio();
            break;
        }
        case QMetaType::QPixmap: {
            QPixmap pixmap = qvariant_cast<QPixmap>(value);
            option->icon = QIcon(pixmap);
            option->decorationSize = pixmap.size() / pixmap.devicePixelRatio();
            break;
        }
        default:
            break;
        }
    }
}

void EmoticonsPageDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!index.data(EmoticonsPageModel::HasContent).toBool())
        return;

    QStyleOptionViewItem opt = option;
    initStyleOption(&opt, index);

    const QWidget *widget = option.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, widget);

    // draw decoration
    QRect cr = style->subElementRect(QStyle::SE_ItemViewItemText, &opt, widget);
    cr = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, opt.decorationSize, cr);
    if (!opt.icon.isNull())
        opt.icon.paint(painter, cr, Qt::AlignCenter, QIcon::Normal);
}
