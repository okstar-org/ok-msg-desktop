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
#include <QApplication>
#include <QToolButton>
#include <QPainter>
#include <QButtonGroup>

#include <math.h>

constexpr int emoji_layout_rows = 8;
constexpr int emoji_layout_cols = 8;
constexpr int emoji_layout_spacing = 4;

class PageDotButton : public QRadioButton {
  public:
    using QRadioButton::QRadioButton;
    QSize sizeHint() const override {
        QSize s = QRadioButton::sizeHint();
        return QSize(s.height(), s.height());
    }
};

EmoticonsWidget::EmoticonsWidget(QWidget* parent)
    : QMenu(parent)
{
    setStyleSheet(Style::getStylesheet("emoticonWidget/emoticonWidget.css"));
    setLayout(&layout);
    layout.addWidget(&stack);

    QWidget* pageButtonsContainer = new QWidget;
    pageButtonsContainer->setObjectName("PageIndexContainer");
    QHBoxLayout *buttonLayout = new QHBoxLayout(pageButtonsContainer);
    pageIndexGroup = new QButtonGroup(this);

    layout.addWidget(pageButtonsContainer);

    const int maxCols = 8;
    const int maxRows = 8;
    const int itemsPerPage = emoji_layout_rows * emoji_layout_cols;

    const QList<QStringList>& emoticons = SmileyPack::getInstance().getEmoticons();
    int itemCount = emoticons.size();
    int pageCount = ceil(float(itemCount) / float(itemsPerPage));

    // respect configured emoticon size
    const int px = Settings::getInstance().getEmojiFontPointSize();
    const QSize size(px, px);

    // create pages
    buttonLayout->addStretch();
    for (int i = 0; i < pageCount; ++i) {
        EmoticonsPageView *page = new EmoticonsPageView(this);
        page->setRange(itemsPerPage * i, std::min(itemsPerPage * (i + 1), itemCount) - 1);
        stack.addWidget(page);
        connect(page, &EmoticonsPageView::clicked, this, [this](int index) {
            const auto emoticons = SmileyPack::getInstance().getEmoticons();
            onSmileyClicked(emoticons.at(index).at(0));
        });

        // page buttons are only needed if there is more than 1 page
        if (pageCount > 1) {
            QRadioButton *pageButton = new PageDotButton(pageButtonsContainer);
            pageButton->setCheckable(true);
            pageButton->setProperty("pageIndex", i);
            pageButton->setCursor(Qt::PointingHandCursor);
            pageButton->setChecked(i == 0);
            buttonLayout->addWidget(pageButton);
            pageIndexGroup->addButton(pageButton);

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
    QWidget *sender = qobject_cast<QAbstractButton *>(QObject::sender());
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
    QList<QAbstractButton *> pageButtons = pageIndexGroup->buttons();
    foreach (QAbstractButton *t_pageButton, pageButtons) {
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

EmoticonsPageView::EmoticonsPageView(QWidget *parent) 
    : QWidget(parent)
{
    setMouseTracking(true);

    int s = Settings::getInstance().getEmojiFontPointSize();
    invisible_button = new QToolButton(this);
    invisible_button->setObjectName("emoticonsItem");
    invisible_button->setVisible(false);
    invisible_button->setIconSize(QSize(s, s));
    invisible_button->setToolButtonStyle(Qt::ToolButtonIconOnly);
}

QSize EmoticonsPageView::sizeHint() const
{
    return minimumSizeHint();
}

QSize EmoticonsPageView::minimumSizeHint() const
{
    QMargins m = this->contentsMargins();
    int baseSize = itemSize();
    int w = (baseSize + emoji_layout_spacing) * emoji_layout_cols - emoji_layout_spacing;
    int h = (baseSize + emoji_layout_spacing) * emoji_layout_rows - emoji_layout_spacing;
    return QSize(w + m.top() + m.bottom(), h + m.left() + m.right());
}

void EmoticonsPageView::setRange(int start, int end)
{
    this->start = std::max(start, 0);
    this->end = std::max(end, 0);
    if (this->end < this->start)
        std::swap(this->start, this->end);

    const auto emoticons = SmileyPack::getInstance().getEmoticons();
    for (int i = this->start; i <= this->end && i < emoticons.count(); i++)
    {
        displayCache.append(emoticons.at(i).at(0));
    }
    updateGeometry();
}

void EmoticonsPageView::mouseMoveEvent(QMouseEvent *event)
{
    int index = indexAtPostion(event->pos());
    if (index != hoverIndex)
    {
        if (hoverIndex >= 0)
            updateIndex(hoverIndex);

        hoverIndex = index;

        if (hoverIndex >= 0)
            updateIndex(hoverIndex);
    }
}

void EmoticonsPageView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        pressedIndex = indexAtPostion(event->pos());
    }
}

void EmoticonsPageView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        int index = indexAtPostion(event->pos());
        if (pressedIndex != index)
            pressedIndex = -1;
    }
}

void EmoticonsPageView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        int index = indexAtPostion(event->pos());
        if (pressedIndex == index && pressedIndex >= 0)
        {
            int tmp = pressedIndex + start;

            const auto emoticons = SmileyPack::getInstance().getEmoticons();
            if (tmp < emoticons.count())
                emit clicked(tmp);
        }
    }
}

void EmoticonsPageView::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    QRect dirty = event->rect();
    // 非精确匹配需要避免越界
    int first = qBound(0, indexAtPostion(dirty.topLeft(), false), displayCache.count() - 1);
    int last = qBound(0, indexAtPostion(dirty.bottomRight(), false), displayCache.count() - 1);

    QPainter painter(this);
    if (first == last)
    {
        drawCell(&painter, first);
    }
    else
    {
        int row_start = first / emoji_layout_cols;
        int row_end = last / emoji_layout_cols;
        int col_start = first % emoji_layout_cols;
        int col_end = last % emoji_layout_cols;
        for (int r = row_start; r <= row_end; r++) {
            for (int c = col_start; c <= col_end; c++)
            {
                int i = r *emoji_layout_cols + c;
                drawCell(&painter, i);
            }
        }
    }
}

int EmoticonsPageView::indexAtPostion(const QPoint &pos, bool accurately)
{
    QRect rc = this->contentsRect();
    if (accurately && !rc.contains(pos)) {
        return -1;
    }

    int x = std::max(pos.x(), 0);
    int y = std::max(pos.y(), 0);
    const int grid_w = itemSize() + emoji_layout_spacing;
    int row = (y - rc.top()) / grid_w;
    int col = (x - rc.left()) / grid_w;
    if (accurately) {
        if (y - rc.top() < ((row + 1) * grid_w - emoji_layout_spacing)) {
            if (x - rc.right() < ((col + 1) * grid_w - emoji_layout_spacing)) {
                return row * emoji_layout_cols + col;
            }
        }
        return -1;
    }
    return row * emoji_layout_cols + col;
}

void EmoticonsPageView::updateIndex(int index) {
    if (index < 0 || index > (end - start) || index >= emoji_layout_rows * emoji_layout_cols)
        return;
    update(indexRect(index));
}

QRect EmoticonsPageView::indexRect(int index)
{
    int baseSize = itemSize();
    int row = index / emoji_layout_cols;
    int col = index % emoji_layout_cols;
    const int grid_w = baseSize + emoji_layout_spacing;
    QRect rc = this->contentsRect();
    return QRect(col * grid_w + rc.left(), row * grid_w + rc.top(), baseSize, baseSize);
}

void EmoticonsPageView::drawCell(QPainter *painter, int index)
{
    QStyleOptionToolButton opt;
    opt.initFrom(invisible_button);
    opt.subControls = QStyle::SC_ToolButton;
    opt.activeSubControls = QStyle::SC_None;
    opt.features = QStyleOptionToolButton::None;
    opt.toolButtonStyle = Qt::ToolButtonIconOnly;
    opt.iconSize = invisible_button->iconSize();

    opt.rect = indexRect(index);
    if (hoverIndex == index)
        opt.state |= QStyle::State_MouseOver;
    if (pressedIndex == index)
        opt.state |= QStyle::State_Sunken;

    opt.icon = *SmileyPack::getInstance().getAsIcon(displayCache.at(index));
    painter->save();
    painter->translate(0.5, 0.5);
    style()->drawComplexControl(QStyle::CC_ToolButton, &opt, painter, invisible_button);
    painter->restore();
}

int EmoticonsPageView::itemSize() const
{
    if (_itemSize < 0)
        _itemSize = invisible_button->sizeHint().width();
    return _itemSize;
}
