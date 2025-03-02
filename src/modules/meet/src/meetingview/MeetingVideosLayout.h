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

#include <QMap>
#include <QScrollArea>
#include <QWidget>
#include "MeetingVideoDefines.h"

class QSplitter;
class QPushButton;

namespace module::meet {

class MeetingVideosLayout;
class MeetingParticipant;
class MeetingVideoOutput;

class MeetingVideosContainer : public QWidget {
    Q_OBJECT
public:
    MeetingVideosContainer(QWidget* parent);
    void resetLayout(VideoLayoutType type);
    VideoLayoutType currentLayoutType() const;
    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void addParticipant(MeetingParticipant* user);
    void removeParticipant(MeetingParticipant* user);
    void clearParticipant();

private:
    void doResetLayout();
    MeetingVideoOutput* getCenterVideo();

private:
    VideoLayoutType layoutType = GridView;

    MeetingVideosLayout* participantLayout = nullptr;
    MeetingVideoOutput* centerVideo = nullptr;
    QSplitter* splitter = nullptr;
};

class MeetingVideosLayout : public QWidget {
    Q_OBJECT
public:
    enum class LayoutType { Grid, Horizontal, Vertical };

public:
    MeetingVideosLayout(LayoutType type, QWidget* parent);
    void setLayoutType(MeetingVideosLayout::LayoutType type, int pageCellCount);
    void setPageCellCount(int count);
    void addParticipant(MeetingParticipant* participant);
    void removeParticipant(MeetingParticipant* participant);
    void clearParticipant();
    void selectParticipant(MeetingParticipant* participant);

private:
    void doLayout();
    void doGridLayout(int cols);
    void nextPage();
    void previousPage();

    void rebindVideos();
    void updateButtonState();
    void updateButtonGeo();
    void updateButtonIcon();
    int recalcPageCount();

    bool event(QEvent* e);

private:
    int cellCount = 1;

    QList<MeetingParticipant*> allParticipant;
    QList<MeetingVideoOutput*> cellVideos;
    MeetingParticipant* currentParticipant = nullptr;
    LayoutType _type = LayoutType::Grid;

    int pageIndex = 0;
    int pageCount = 1;

    QPushButton* nextPageButton = nullptr;
    QPushButton* prevPageButton = nullptr;

    friend class MeetingVideosContainer;
};
}  // namespace module::meet
#endif  // !MEETINGVIDEOSLAYOUT_H
