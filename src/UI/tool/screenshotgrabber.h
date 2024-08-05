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

#ifndef SCREENSHOTGRABBER_H
#define SCREENSHOTGRABBER_H

#include <QPixmap>
#include <QPointer>

class QGraphicsSceneMouseEvent;
class QGraphicsPixmapItem;
class QGraphicsRectItem;
class QGraphicsTextItem;
class QGraphicsScene;
class QGraphicsView;
class QKeyEvent;
class ScreenGrabberChooserRectItem;
class ScreenGrabberOverlayItem;
class ToolBoxGraphicsItem;

class ScreenshotGrabber : public QObject {
    Q_OBJECT
public:
    ScreenshotGrabber();
    ~ScreenshotGrabber() override;

    bool eventFilter(QObject* object, QEvent* event) override;

    void showGrabber();

public slots:
    void acceptRegion();
    void reInit();

signals:
    void screenshotTaken(const QPixmap& pixmap);
    void regionChosen(QRect region);
    void rejected();

private:
    friend class ScreenGrabberOverlayItem;
    bool mKeysBlocked;

    void setupScene();

    void useNothingSelectedTooltip();
    void useRegionSelectedTooltip();
    void chooseHelperTooltipText(QRect rect);
    void adjustTooltipPosition();

    bool handleKeyPress(QKeyEvent* event);
    void reject();

    QPixmap grabScreen();

    void hideVisibleWindows();
    void restoreHiddenWindows();

    void beginRectChooser(QGraphicsSceneMouseEvent* event);

private:
    QPixmap screenGrab;
    QGraphicsScene* scene;
    QGraphicsView* window;
    QGraphicsPixmapItem* screenGrabDisplay;
    ScreenGrabberOverlayItem* overlay;
    ScreenGrabberChooserRectItem* chooserRect;
    ToolBoxGraphicsItem* helperToolbox;
    QGraphicsTextItem* helperTooltip;

    qreal pixRatio = 1.0;

    bool mQToxVisible;
    QVector<QPointer<QWidget>> mHiddenWindows;
};

#endif  // SCREENSHOTGRABBER_H
