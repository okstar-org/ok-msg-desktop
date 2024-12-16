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

#include "MeetingVideosLayout.h"
#include <QDebug>
#include <QEvent>
#include <QPushButton>
#include <QSplitter>
#include <QStyle>
#include <QVBoxLayout>
#include <QVariant>
#include "../MeetingParticipant.h"
#include "MeetingVideoOutput.h"

namespace module::meet {

MeetingVideosContainer::MeetingVideosContainer(QWidget* parent) : QWidget(parent) {
    setAttribute(Qt::WA_StyledBackground);

    splitter = new QSplitter(Qt::Vertical, this);
    participantLayout = new MeetingVideosLayout(MeetingVideosLayout::LayoutType::Grid, this);
    participantLayout->setObjectName("videoList");
    participantLayout->setPageCellCount(2);
    splitter->addWidget(participantLayout);
    doResetLayout();

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(splitter);
}

void MeetingVideosContainer::resetLayout(VideoLayoutType type) {
    if (layoutType == type) {
        return;
    }
    layoutType = type;
    switch (type) {
        case module::meet::VideoLayoutType::GridView:
            participantLayout->setPageCellCount(9);
            participantLayout->setLayoutType(MeetingVideosLayout::LayoutType::Grid, 9);
            break;
        case module::meet::VideoLayoutType::TopList:
            participantLayout->setLayoutType(MeetingVideosLayout::LayoutType::Horizontal, 4);
            break;
        case module::meet::VideoLayoutType::RightList:
            participantLayout->setLayoutType(MeetingVideosLayout::LayoutType::Vertical, 4);
            break;
        default:
            break;
    }
    doResetLayout();
}

module::meet::VideoLayoutType MeetingVideosContainer::currentLayoutType() const {
    return layoutType;
}

QSize MeetingVideosContainer::sizeHint() const {
    return QSize(600, 400);
}

QSize MeetingVideosContainer::minimumSizeHint() const {
    return QSize(300, 200);
}

void MeetingVideosContainer::addParticipant(MeetingParticipant* participant) {
    participantLayout->addParticipant(participant);
}

void MeetingVideosContainer::removeParticipant(MeetingParticipant* participant) {
    participantLayout->removeParticipant(participant);
}

void MeetingVideosContainer::clearParticipant() {
    participantLayout->clearParticipant();
}

void MeetingVideosContainer::doResetLayout() {
    if (layoutType == module::meet::VideoLayoutType::GridView) {
        if (splitter->indexOf(centerVideo) >= 0) {
            centerVideo->setParent(this);
            centerVideo->hide();
        }
        if (splitter->indexOf(participantLayout) < 0) {
            splitter->addWidget(participantLayout);
            splitter->setStretchFactor(splitter->count() - 1, 1);
        }
    } else if (layoutType == module::meet::VideoLayoutType::TopList) {
        auto center = getCenterVideo();
        splitter->setOrientation(Qt::Vertical);
        splitter->insertWidget(1, center);
        splitter->setStretchFactor(0, 0);
        splitter->setStretchFactor(1, 1);
        if (!center->isVisible()) {
            center->setVisible(true);
        }
        splitter->setSizes({120, -1});
    } else if (layoutType == module::meet::VideoLayoutType::RightList) {
        auto center = getCenterVideo();
        splitter->setOrientation(Qt::Horizontal);
        splitter->insertWidget(0, center);
        splitter->setStretchFactor(0, 1);
        splitter->setStretchFactor(1, 0);
        if (!center->isVisible()) {
            center->setVisible(true);
        }
        splitter->setSizes({-1, 180});
    }
}

MeetingVideoOutput* MeetingVideosContainer::getCenterVideo() {
    if (!centerVideo) {
        centerVideo = new MeetingVideoOutput(this);
        centerVideo->setObjectName("centerVideo");
    }
    return centerVideo;
}

constexpr int grid_max_cols = 5;
constexpr int grid_max_videos = grid_max_cols * grid_max_cols;

MeetingVideosLayout::MeetingVideosLayout(LayoutType type, QWidget* parent)
        : QWidget(parent), _type(type) {
    setAttribute(Qt::WA_StyledBackground);
    setAttribute(Qt::WA_Hover);

    nextPageButton = new QPushButton(this);
    nextPageButton->setObjectName("nextPage");
    nextPageButton->setVisible(false);
    nextPageButton->setCursor(Qt::PointingHandCursor);
    prevPageButton = new QPushButton(this);
    prevPageButton->setObjectName("prevPage");
    prevPageButton->setVisible(false);
    prevPageButton->setCursor(Qt::PointingHandCursor);
    connect(nextPageButton, &QPushButton::clicked, this, &MeetingVideosLayout::nextPage);
    connect(prevPageButton, &QPushButton::clicked, this, &MeetingVideosLayout::previousPage);

    updateButtonIcon();
}

void MeetingVideosLayout::setLayoutType(MeetingVideosLayout::LayoutType type, int pageCellCount) {
    if (pageCellCount != this->cellCount) {
        if (_type != type) {
            _type = type;
            updateButtonIcon();
        }
        setPageCellCount(pageCellCount);
    } else if (_type != type) {
        pageIndex = 0;
        _type = type;
        doLayout();
        rebindVideos();
        updateButtonIcon();
    }
}

void MeetingVideosLayout::setPageCellCount(int count) {
    if (cellCount == count) {
        return;
    }
    // 最大显示25个
    cellCount = std::min(std::max(1, count), grid_max_videos);
    if (cellVideos.count() > cellCount) {
        int diff = cellVideos.count() - cellCount;
        for (int i = 0; i < diff; i++) {
            cellVideos.takeLast()->deleteLater();
        }
    } else if (cellVideos.count() < cellCount) {
        int diff = cellCount - cellVideos.count();
        for (int i = 0; i < diff; i++) {
            cellVideos.append(new MeetingVideoOutput(this));
        }
    }
    pageIndex = 0;
    pageCount = recalcPageCount();
    doLayout();
    rebindVideos();
    updateButtonState();
}

void MeetingVideosLayout::addParticipant(MeetingParticipant* participant) {
    if (allParticipant.contains(participant)) {
        return;
    }

    allParticipant.append(participant);
    pageCount = recalcPageCount();
    if (pageIndex == pageCount - 1) {
        rebindVideos();
    }
    updateButtonState();
}

void MeetingVideosLayout::removeParticipant(MeetingParticipant* participant) {
    if (!allParticipant.contains(participant)) {
        return;
    }

    allParticipant.removeAll(participant);
    pageCount = recalcPageCount();

    if (pageIndex >= pageCount - 1) {
        pageIndex = pageCount - 1;
        rebindVideos();
    }
    updateButtonState();
}

void MeetingVideosLayout::clearParticipant() {
    for (int index = 0; index < cellVideos.count(); index++) {
        auto output = cellVideos.at(index);
        output->bindParticipant(nullptr);
    }
    allParticipant.clear();
}

void MeetingVideosLayout::doLayout() {
    if (!this->size().isValid()) {
        return;
    }

    if (_type == LayoutType::Grid) {
        int prefer_col = 1;
        for (int i = 1; i <= grid_max_cols; i++) {
            if (i * i >= cellCount) {
                prefer_col = i;
                break;
            }
        }
        doGridLayout(prefer_col);
    } else if (_type == LayoutType::Horizontal) {
        doGridLayout(cellCount);
    } else if (_type == LayoutType::Vertical) {
        doGridLayout(1);
    }
}

void MeetingVideosLayout::doGridLayout(int cols) {
    Q_ASSERT(cols >= 1);
    int row_count = (cellCount + cols - 1) / cols;
    Q_ASSERT(row_count >= 1);

    const int spacing = 3;

    qreal grid_w = (width() - spacing * (cols - 1)) / static_cast<qreal>(cols);
    qreal grid_h = (height() - spacing * (row_count - 1)) / static_cast<qreal>(row_count);

    int index = 0;
    for (MeetingVideoOutput* output : cellVideos) {
        int r = index / cols;
        int c = index % cols;
        output->setGeometry(QRect(c * (grid_w + spacing), r * (grid_h + spacing), grid_w, grid_h));
        index++;
    }
}

void MeetingVideosLayout::nextPage() {
    if (pageIndex >= pageCount - 1) {
        pageIndex = pageCount - 1;
        return;
    }
    pageIndex++;
    rebindVideos();
    updateButtonState();
}

void MeetingVideosLayout::previousPage() {
    if (pageIndex <= 0) {
        pageIndex = 0;
        return;
    }
    pageIndex--;
    rebindVideos();
    updateButtonState();
}

void MeetingVideosLayout::rebindVideos() {
    int offset = pageIndex * cellCount;
    for (int index = 0; index < cellVideos.count(); index++) {
        auto output = cellVideos.at(index);
        MeetingParticipant* participant = allParticipant.value(index + offset);
        output->bindParticipant(participant);
        if (participant) {
            if (!output->isVisibleTo(this)) {
                output->setVisible(true);
            }
        } else {
            if (output->isVisibleTo(this)) {
                output->setVisible(false);
            }
        }
    }
}

void MeetingVideosLayout::updateButtonState() {
    nextPageButton->setVisible(pageCount > 1 && pageIndex < pageCount - 1);
    prevPageButton->setVisible(pageCount > 1 && pageIndex > 0);
}

void MeetingVideosLayout::updateButtonGeo() {
    if (!this->size().isValid()) {
        return;
    }

    QRect rect = this->rect();

    QSize btn_size = nextPageButton->sizeHint();
    if (this->_type == LayoutType::Vertical) {
        btn_size.transpose(); //交换
    }
    if (_type == LayoutType::Grid || _type == LayoutType::Horizontal) {
        QRect prev_rect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignLeft | Qt::AlignVCenter,
                                              btn_size, rect);
        QRect next_rect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignRight | Qt::AlignVCenter,
                                              btn_size, rect);
        prevPageButton->setGeometry(prev_rect);
        nextPageButton->setGeometry(next_rect);
    } else if (_type == LayoutType::Vertical) {
        QRect prev_rect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignTop | Qt::AlignHCenter,
                                              btn_size, rect);
        QRect next_rect = QStyle::alignedRect(Qt::LeftToRight, Qt::AlignBottom | Qt::AlignHCenter,
                                              btn_size, rect);
        prevPageButton->setGeometry(prev_rect);
        nextPageButton->setGeometry(next_rect);
    }
    prevPageButton->raise();
    nextPageButton->raise();
}

void MeetingVideosLayout::updateButtonIcon() {
    if (_type == MeetingVideosLayout::LayoutType::Vertical) {
        nextPageButton->setIcon(QIcon(":/meet/image/arrow_down.svg"));
        prevPageButton->setIcon(QIcon(":/meet/image/arrow_up.svg"));
    } else {
        nextPageButton->setIcon(QIcon(":/meet/image/arrow_right.svg"));
        prevPageButton->setIcon(QIcon(":/meet/image/arrow_left.svg"));
    }
    updateButtonGeo();
}

int MeetingVideosLayout::recalcPageCount() {
    return std::max(1, (allParticipant.count() + cellCount - 1) / cellCount);
}

bool MeetingVideosLayout::event(QEvent* e) {
    bool ret = QWidget::event(e);
    switch (e->type()) {
        case QEvent::Resize:
            doLayout();
            updateButtonGeo();
            break;
        case QEvent::HoverEnter:
            if (pageCount > 1) {
                nextPageButton->setVisible(true);
                prevPageButton->setVisible(true);
            }
            break;
        case QEvent::HoverLeave:
        case QEvent::Leave:
            if (pageCount > 1) {
                nextPageButton->setVisible(false);
                prevPageButton->setVisible(false);
            }
            break;
        default:
            break;
    }
    return ret;
}
}  // namespace module::meet