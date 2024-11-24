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

#ifndef MEETINGVIDEOSLAYOUT_H
#define MEETINGVIDEOSLAYOUT_H

#include <QWidget>
#include <QScrollArea>
#include "MeetingVideoDefines.h"

class MeetingVideosLayout;
class MeetingParticipant;
class MeetingVideoOutput;
class QSplitter;

class MeetingVideosContainer : public QWidget {
public:
    MeetingVideosContainer(QWidget* parent);
    void resetLayout(module::meet::VideoLayoutType type);
    module::meet::VideoLayoutType currentLayoutType() const;
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void addParticipant(MeetingParticipant * participant);

private:
    void doResetLayout();
    MeetingVideoOutput * getCenterVideo();

private:
    module::meet::VideoLayoutType layoutType = module::meet::GridView;

    MeetingVideosLayout* participantLayout = nullptr;
    MeetingVideoOutput* centerVideo = nullptr;
    QSplitter* splitter = nullptr;
};

class QPushButton;
class MeetingVideosLayout : public QWidget {
public:
    enum class LayoutType {Grid, Horizontal, Vertical };

public:
    MeetingVideosLayout(LayoutType type, QWidget* parent);
    void setLayoutType(MeetingVideosLayout::LayoutType type, int pageCellCount);
    void setPageCellCount(int count);
    void addParticipant(MeetingParticipant* participant);
    void removeParticipant(MeetingParticipant* participant);
    
private:
    void doLayout();
    void doGridLayout(int cols);
    void nextPage();
    void previousPage();

    void rebindVideos();
    void updateButtonState();
    void updateButtonGeo();
    int recalcPageCount();

    bool event(QEvent* e);

private:
    int cellCount = 1;
    QList<MeetingParticipant*> allParticipant;
    QList<MeetingVideoOutput*> cellVideos;
    LayoutType _type = LayoutType::Grid;

    int pageIndex = 0;
    int pageCount = 1;

    QPushButton* nextPageButton = nullptr;
    QPushButton* prevPageButton = nullptr;

    friend class MeetingVideosContainer;
};

#endif  // !MEETINGVIDEOSLAYOUT_H
