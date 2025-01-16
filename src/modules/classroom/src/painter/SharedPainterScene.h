
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

#pragma once

#include <stdio.h>
#include <string.h>

#include <QColor>
#include <QFileIconProvider>
#include <QImage>
#include <QList>
#include <QObject>
#include <QPainter>
#include <QPixmap>
#include <QPointF>
#include <QRectF>
#include <QString>

#include <QGraphicsItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QTemporaryFile>

#include "src/base/timer.h"

#include "lib/network/NetworkHttp.h"

#include "OkGraphicsTextItem.h"
#include "PaintItem.h"
#include "PainterView.h"
#include "WhiteboardController.h"
#include "lib/backend/BaseService.h"

namespace module::classroom {

class ICanvasViewEvent;
class CSharedPaintCommandManager;

class CSharedPainterScene : public QGraphicsScene, public IGluePaintCanvas {
    Q_OBJECT
public:
    static const int ZVALUE_NORMAL = 1;
    static const int ZVALUE_TOPMOST = 9999;

    CSharedPainterScene(ICanvasViewEvent* evt);

    ~CSharedPainterScene();

    void setCommandMgr(std::shared_ptr<CSharedPaintCommandManager> commandMgr) {
        commandMgr_ = (commandMgr);
    }

    void setCursor(QCursor cursor) {
        QList<QGraphicsView*> list = views();
        for (int i = 0; i < list.size(); ++i) {
            list.at(i)->setCursor(cursor);
        }
    }

    bool freezeAction(void) {
        if (freezeActionFlag_) return false;

        // TODO : CURSOR CHANGE ON DISABLE WIDGET
        // When this view become disabled state, the cursor has changed to default
        // cursor automatically.. I don't know how to avoid this problem. SO I call
        // QApplication::setOverrideCursor()...

        // setCursor(
        // QCursor(QPixmap(":/SharedPainter/Resources/draw_disabled.png")) );

        QList<QGraphicsView*> list = views();
        for (int i = 0; i < list.size(); ++i) {
            list.at(i)->setEnabled(false);
        }

        freezeActionFlag_ = true;
        return true;
    }

    bool thawAction(void) {
        if (!freezeActionFlag_) return false;

        freezeActionFlag_ = false;

        QList<QGraphicsView*> list = views();
        for (int i = 0; i < list.size(); ++i) {
            list.at(i)->setEnabled(true);
        }
        return true;
    }

    void setPenColor(const QColor& clr) {
        penClr_ = clr;
    }

    void setPenWidth(int width) {
        penWidth_ = width;
    }

    void setTextColor(const QColor& clr) {
        textClr_ = clr;
    }

    void setTextWidth(int width) {
        textWidth_ = width;
    }

    void updateBackground(void) {
        resetBackground(sceneRect());
    }

    int backgroundGridLineSize(void) {
        return gridLineSize_;
    }

    int penWidth(void) {
        return penWidth_;
    }

    const QColor& penColor(void) {
        return penClr_;
    }

    const QColor& backgroundColor(void) {
        return backgroundColor_;
    }

    bool isSettingShowLastAddItemBorder(void) {
        return showLastAddItemBorderFlag_;
    }

    void setSettingShowLastAddItemBorder(bool enable) {
        if (showLastAddItemBorderFlag_ == enable) return;

        showLastAddItemBorderFlag_ = enable;
        if (enable)
            drawLastItemBorderRect();
        else
            clearLastItemBorderRect();
    }

    void drawLastItemBorderRect(void);

    void clearSelectedItemState(void) {
        QList<QGraphicsItem*> list = selectedItems();
        for (int i = 0; i < list.size(); ++i) {
            list.at(i)->setSelected(false);
        }
    }

    void setHighQualityMoveItems(bool enabled = true) {
        hiqhQualityMoveItemMode_ = enabled;
    }

    bool isHighQualityMoveItems(void) {
        return hiqhQualityMoveItemMode_;
    }

    QGraphicsItem* findById(ItemId id);

public:
    // IGluePaintCanvas
    QRectF itemBoundingRect(std::shared_ptr<CPaintItem> item);

    virtual void moveItem(std::shared_ptr<CPaintItem> item, double x, double y);

    virtual void updateItem(std::shared_ptr<CPaintItem> item);

    void removeItem(QGraphicsItem* item);

    void removePaintItem(CPaintItem* item) override;

    virtual void removeInvalidItems();

    virtual void drawSendingStatus(std::shared_ptr<CPaintItem> item);

    virtual void drawLine(std::shared_ptr<CLineItem> line);

    virtual void drawFile(std::shared_ptr<CFileItem> file);

    virtual void drawText(std::shared_ptr<CTextItem> text);

    virtual void drawImage(std::shared_ptr<CImageItem> image);

    virtual void drawImageFile(std::shared_ptr<CImageFileItem> imageFile);

    virtual void drawBackgroundGridLine(int size);

    virtual void drawBackgroundImage(std::shared_ptr<CBackgroundImageItem> image);

    virtual void clearBackgroundImage(void);

    virtual void clearScreen(void) {
        // lastAddItem_ = std::shared_ptr<CPaintItem>();
        // gridLineSize_ = 0;
        // backgroundColor_ = Qt::transparent;
        // currentZValue_ = ZVALUE_NORMAL;
        // clearLastItemBorderRect();
        // clearBackgroundImage();
    }

    virtual void setBackgroundColor(int r, int g, int b, int a);

private slots:

    void sceneRectChanged(const QRectF& rect);

    void onTimer(void);

    void onScreenCapture(QPixmap qPixmap);

    void onCtrollerChecked(WB_CTRL ctrl, bool isChecked);

    // QGraphicsScene
private:
    void drawBackground(QPainter* painter, const QRectF& rect);

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent* evt);
    void mousePressEvent(QGraphicsSceneMouseEvent* evt);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* evt);

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    void dragEnterEvent(QGraphicsSceneDragDropEvent* evt);
    void dragLeaveEvent(QGraphicsSceneDragDropEvent* evt);
    void dragMoveEvent(QGraphicsSceneDragDropEvent* evt);
    void dropEvent(QGraphicsSceneDragDropEvent* evt);

public:
    void onItemMoving(std::shared_ptr<CPaintItem>, const QPointF&);

    void onItemUpdate(std::shared_ptr<CPaintItem>);

    void onItemRemove(std::shared_ptr<CPaintItem>);

    void onItemClipboardCopy(std::shared_ptr<CPaintItem>);

    void setToolType(ToolboxType toolboxType);

    ToolboxType toolType();

    void addGeneralUrlItem(const QPointF& pos, const lib::backend::FileResult& urlInfo);

private:
    void onClipboardPaste();

    qreal currentZValue(void) {
        currentZValue_ += 0.01;
        return currentZValue_;
    }

    void startBlinkLastItem(void);

    void resetBackground(const QRectF& rect);

    std::shared_ptr<CPaintItem> findPaintItem(QGraphicsItem* item);

    void clearLastItemBorderRect(void);

    void addImageFileItem(const QPointF& pos, const QString& path);

    void addGeneralFileItem(const QPointF& pos, const QString& path);

    void resizeImage(QImage* image, const QSize& newSize);

    void drawLineStart(const QPointF& pt, const QColor& clr, int width);

    void drawLineTo(const QPointF& pt1, const QPointF& pt2, const QColor& clr, int width);

    void doLowQualityMoveItems(void);

    void setScaleImageFileItem(std::shared_ptr<CImageFileItem> image,
                               QGraphicsPixmapItem* pixmapItem);

    void commonAddItem(std::shared_ptr<CPaintItem> item, QGraphicsItem* drawingItem,
                       int borderType);

    void internalDrawGridLine(QPainter* painter, const QRectF& rect, int gridLineSize);

    void fireEvent_DrawItem(std::shared_ptr<CPaintItem> item);

    void endOfDrawText();

    std::shared_ptr<CTextItem> createTextItem(const QString& s = "");

private:
    bool _disabled = false;

    ICanvasViewEvent* eventTarget_;

    QColor penClr_;
    int penWidth_;

    QColor textClr_;
    int textWidth_;

    QPointF prevPos_;

    bool freezeActionFlag_;
    bool drawFlag_;

    ToolboxType _toolboxType;

    bool hiqhQualityMoveItemMode_;

    QImage image_;
    QPixmap backgroundPixmap_;

    std::shared_ptr<CSharedPaintCommandManager> commandMgr_;

    std::shared_ptr<CBackgroundImageItem> backgroundImageItem_;

    std::shared_ptr<CLineItem> currLineItem_;
    std::shared_ptr<CTextItem> currTextItem_;

    std::vector<QGraphicsItem*> tempLineItemList_;
    ITEM_SET tempMovingItemList_;

    QFileIconProvider fileIconProvider_;
    qreal currentZValue_;
    qreal currentLineZValue_;
    QColor backgroundColor_;
    int gridLineSize_;

    // delayCaller
    std::shared_ptr<base::DelayedCallTimer> delayCaller_;

    // blink last item

    QTimer* timer_;
    QGraphicsItem* lastCoverGraphicsItem_;
    qint64 lastTimeValue_;
    int timeoutRemoveLastCoverItem_;
    int lastItemBorderType_;
    std::shared_ptr<CPaintItem> lastAddItem_;
    bool lastTempBlinkShowFlag_;
    bool showLastAddItemBorderFlag_;

    std::unique_ptr<lib::network::NetworkHttp> m_networkManager;

    // 控制面板
    WhiteboardController* _controller = nullptr;

    std::recursive_mutex text_mutex_;

    std::recursive_mutex item_mutex_;

    QTemporaryFile temp_file_;

    void removeSelections();
signals:

    void receivedDropFile(QString);

    void receivedScreenCapture(QPixmap);

    void openUrl(lib::backend::FileResult);

    void openFile(std::shared_ptr<CFileItem>);

    void dragLeaveEvent();

    void dropEvent();

public slots:

    void editorLostFocus(OkGraphicsTextItem* item);

    void onItemChanged(OkGraphicsTextItem* item,
                       QGraphicsTextItem::GraphicsItemChange change,
                       const QVariant& value);
};

}  // namespace module::classroom
