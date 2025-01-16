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

#include "SharedPainterScene.h"
#include "PaintItem.h"
#include "PainterView.h"

#include <QBrush>
#include <QClipboard>
#include <QColor>
#include <QDateTime>
#include <QDebug>
#include <QImage>
#include <QKeyEvent>
#include <QList>
#include <QMimeData>
#include <QMouseEvent>
#include <QObject>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPixmap>
#include <QPointF>
#include <QRectF>
#include <QSize>
#include <QString>
#include <QTimer>
#include <QVariant>
#include <QWidget>
#include <memory>

#include <QAbstractGraphicsShapeItem>
#include <QBuffer>
#include <QGraphicsEllipseItem>
#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsSceneDragDropEvent>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsView>
#include <QStyleOptionGraphicsItem>
#include <QTemporaryFile>
#include <QTextCursor>

#include "src/base/logs.h"

#include "base/images.h"
#include "lib/network/NetworkHttp.h"

#define DEFAULT_TIMEOUT_REMOVE_LAST_COVER_ITEM 600  // msec
#define DEFAULT_COVER_RECT_OFFSET 5

#define ITEM_DATA_KEY_OWNER 0
#define ITEM_DATA_KEY_ITEMID 1

namespace module::painter {

enum ItemBorderType {
    Border_Ellipse,
    Border_PainterPath,
    Border_Rect,
};

template <class T>  //
class OkGraphicItem : public T {
public:
    OkGraphicItem(CSharedPainterScene* scene) : scene_(scene) {}

    void setItemData(std::weak_ptr<CPaintItem> data) {
        //    T::setAcceptHoverEvents(true);

        itemData_ = data;

        //    if (std::shared_ptr<CPaintItem> r = itemData_.lock()) {
        //
        //      T::setData(ITEM_DATA_KEY_OWNER, QString(r->owner().c_str()));
        //      T::setData(ITEM_DATA_KEY_ITEMID, QString(r->itemId().c_str()));
        //
        //      if (r->type() != PaintItemType::PT_TEXT) {
        //        r->setPos(T::scenePos().x(), T::scenePos().y());
        //      }
        //    }
    }
    //
    //    QVariant itemChange(QGraphicsItem::GraphicsItemChange change,
    //                        const QVariant &value) {
    //      if (change == QGraphicsItem::ItemPositionHasChanged) {
    //        if (std::shared_ptr<CPaintItem> r = itemData_.lock()) {
    //          QPointF newPos = value.toPointF();
    //          scene_->onItemMoving(r, newPos);
    //          return newPos;
    //        }
    //      }
    //      return T::itemChange(change, value);
    //    }

    //    void keyPressEvent(QKeyEvent *event) {
    //      //        if (event->key() == Qt::Key_Delete) {
    //      //            if (std::shared_ptr<CPaintItem> r = itemData_.lock()) {
    //      //                scene_->onItemRemove(r);
    //      //            }
    //      //        }
    //
    //      if (event->key() == Qt::Key_C &&
    //          event->modifiers() == Qt::ControlModifier) {
    //        if (std::shared_ptr<CPaintItem> r = itemData_.lock()) {
    //          scene_->onItemClipboardCopy(r);
    //        }
    //      }
    //    }
    //
    //  void hoverEnterEvent(QGraphicsSceneHoverEvent *event) {
    //    if (scene_->toolType() == ToolboxType::P_PEN)
    //      return;
    //
    //    //        scene_->setCursor(Qt::OpenHandCursor);
    //  }
    //
    //  void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) {
    //    if (scene_->toolType() == ToolboxType::P_PEN)
    //      return;
    //
    //    //        scene_->setCursor(Qt::PointingHandCursor);
    //  }
    //
    //  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) {
    //    if (scene_->toolType() == ToolboxType::P_PEN)
    //      return;
    //
    //    if (std::shared_ptr<CPaintItem> r = itemData_.lock()) {
    //      r->execute();
    //    }
    //    T::mouseDoubleClickEvent(event);
    //  }
    //
    //    void mousePressEvent(QGraphicsSceneMouseEvent *event) {
    //      scene_->setCursor(Qt::ClosedHandCursor);
    //      T::mousePressEvent(event);
    //    }
    //
    //    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    //      scene_->setCursor(Qt::OpenHandCursor);
    //      T::mouseReleaseEvent(event);
    //    }
    //
    //  void wheelEvent(QGraphicsSceneWheelEvent *event) {
    //
    //    qDebug() << ":" << event;
    //
    //    if (scene_->toolType() == ToolboxType::P_PEN)
    //      return;
    //
    //    if (std::shared_ptr<CPaintItem> r = itemData_.lock()) {
    //      double sc = r->scale();
    //      if (event->delta() < 0)
    //        sc -= ITEM_SCALE_STEP;
    //      else
    //        sc += ITEM_SCALE_STEP;
    //
    //      if (sc > ITEM_SCALE_MAX)
    //        return;
    //      if (sc < ITEM_SCALE_MIN)
    //        return;
    //
    //      r->setScale(sc);
    //      scene_->onItemUpdate(r);
    //    }
    //  }

private:
    CSharedPainterScene* scene_;
    std::weak_ptr<CPaintItem> itemData_;
};

CSharedPainterScene::CSharedPainterScene(ICanvasViewEvent* evt)
        : eventTarget_(nullptr)
        , penClr_(Qt::black)
        , penWidth_(2)
        , textClr_(Qt::black)
        , textWidth_(12)
        , freezeActionFlag_(false)
        , drawFlag_(false)
        , hiqhQualityMoveItemMode_(false)
        , currentZValue_(ZVALUE_NORMAL)
        , backgroundColor_(Qt::white)
        , gridLineSize_(0)
        , lastCoverGraphicsItem_(nullptr)
        , timeoutRemoveLastCoverItem_(0)
        , lastTempBlinkShowFlag_(false)
        , showLastAddItemBorderFlag_(false)
        , m_networkManager(std::make_unique<lib::network::NetworkHttp>(this)) {
    // 延时器
    delayCaller_ = (std::make_unique<base::DelayedCallTimer>());

    // 事件处理器
    eventTarget_ = evt;

    // clearScreen();

    // Timer Setting
    timer_ = new QTimer();
    timer_->start(150);
    QObject::connect(timer_, SIGNAL(timeout()), this, SLOT(onTimer()));

    QObject::connect(this, SIGNAL(sceneRectChanged(const QRectF&)), this,
                     SLOT(sceneRectChanged(const QRectF&)));

    //		if (!_controller) {
    //			base::DelayedCallTimer * caller = new
    // base::DelayedCallTimer(); 			caller->call(1800, [=] {
    // UI::page::Classing* classing =
    // qobject_cast<Classing*>(oApp->pageManager()->getPage(UI::page::PageMenu::classing));
    //				_controller =
    // classing->wbLayout()->wbWidget()->painter()->controller();
    //				connect(_controller,
    // SIGNAL(checked(WB_CTRL, bool)), this,
    // SLOT(onCtrollerChecked(WB_CTRL, bool)));
    //			});
    //		}

    //    QObject::connect(this, &CSharedPainterScene::openUrl,
    //    [](lib::backend::FileResult info){
    //        office::FrameWidget* of = office::OfficeFrame::CreateFrame(info);
    //        if(!of){
    //            DEBUG_LOG_S(L_ERROR) << ("can't open for url:") << info.url;
    //            return;
    //        }
    //    });

    //    Painter *painter = Painter::Get();
}

CSharedPainterScene::~CSharedPainterScene() {}

void CSharedPainterScene::fireEvent_DrawItem(std::shared_ptr<CPaintItem> item) {
    if (eventTarget_) eventTarget_->onDrawItem(item);
}

void CSharedPainterScene::endOfDrawText() {
    QList<QGraphicsItem*> list = this->items();
    for (int i = 0; i < list.size(); ++i) {
        QGraphicsItem* _item = (list.at(i));
        QGraphicsTextItem* _t_item = static_cast<QGraphicsTextItem*>(_item);
        if (_t_item) {
            _t_item->setTextInteractionFlags(Qt::TextInteractionFlag::NoTextInteraction);
            _t_item->setFocus(Qt::FocusReason::NoFocusReason);

            if (_t_item->toPlainText().length() == 0) {
                QGraphicsScene::removeItem(_item);
            }
        }
    }
}

painter::ToolboxType CSharedPainterScene::toolType() {
    return this->_toolboxType;
}

void CSharedPainterScene::setToolType(painter::ToolboxType toolboxType) {
    switch (toolboxType) {
        case painter::ToolboxType::P_PEN: {
            _toolboxType = toolboxType;
            QList<QGraphicsView*> list = views();
            for (int i = 0; i < list.size(); ++i) {
                auto view = list.at(i);
                view->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
                view->setDragMode(QGraphicsView::NoDrag);
            }
            //        setCursor(QCursor(QPixmap("://resources/painter/draw_line.png")));
            break;
        }
        case ToolboxType::P_TEXT: {
            _toolboxType = toolboxType;
            break;
        }

        case ToolboxType::P_MOVE: {
            _toolboxType = toolboxType;
            QList<QGraphicsView*> list = views();
            for (int i = 0; i < list.size(); ++i) {
                list.at(i)->setDragMode(QGraphicsView::RubberBandDrag);
            }
            //        setCursor(Qt::PointingHandCursor);
            break;
        }

        case ToolboxType::P_REMOVE: {
            //        setCursor(Qt::ArrowCursor);

            // DELETE
            removeSelections();

            break;
        }
        case ToolboxType::P_CUTTER: {
            break;
        }

        default:
            break;
    }

    //    drawFlag_ = true;

    // 结束原有文本元素。
    //     endOfDrawText();
}

void CSharedPainterScene::onTimer() {
    if (nullptr == lastCoverGraphicsItem_ || timeoutRemoveLastCoverItem_ <= 0) return;

    qint64 now = QDateTime::currentDateTime().toMSecsSinceEpoch();
    qint64 elapsedMSec = now - lastTimeValue_;
    timeoutRemoveLastCoverItem_ -= elapsedMSec;

    if (timeoutRemoveLastCoverItem_ > 0) {
        if (lastTempBlinkShowFlag_)
            lastCoverGraphicsItem_->show();
        else
            lastCoverGraphicsItem_->hide();

        lastTempBlinkShowFlag_ = !lastTempBlinkShowFlag_;
    }

    if (timeoutRemoveLastCoverItem_ <= 0) {
        clearLastItemBorderRect();
    }

    lastTimeValue_ = now;
}

void CSharedPainterScene::sceneRectChanged(const QRectF& rect) {
    // qDebug() << "sceneRectChanged" << rect;
    resetBackground(rect);
}

void CSharedPainterScene::internalDrawGridLine(QPainter* painter,
                                               const QRectF& rect,
                                               int gridLineSize) {
    painter->setPen(QPen(QColor(102, 102, 102), 2, Qt::SolidLine));

    int w = rect.width();
    int h = rect.height();

    // vertical line
    int x = gridLineSize;
    for (; x < w; x += gridLineSize) {
        painter->drawLine(x, 0, x, h);
    }

    // horizontal line
    int y = gridLineSize;
    for (; y < h; y += gridLineSize) {
        painter->drawLine(0, y, w, y);
    }
}

std::shared_ptr<CPaintItem> CSharedPainterScene::findPaintItem(QGraphicsItem* item) {
    if (!item) return std::shared_ptr<CPaintItem>();

    std::string owner = item->data(ITEM_DATA_KEY_OWNER).toString().toStdString();
    ItemId itemId = item->data(ITEM_DATA_KEY_ITEMID).toString().toStdString();

    std::shared_ptr<CPaintItem> _item = commandMgr_->findItem(owner, itemId);
    return _item;
}

void CSharedPainterScene::resetBackground(const QRectF& rect) {
    QImage newImage(rect.toRect().size(), QImage::Format_RGB32);
    newImage.fill(backgroundColor_);

    QPainter painter(&newImage);

    // draw image
    if (!backgroundPixmap_.isNull()) painter.drawPixmap(QPoint(0, 0), backgroundPixmap_);

    // draw grid line
    if (gridLineSize_ > 0) {
        internalDrawGridLine(&painter, rect, gridLineSize_);
    }

    image_ = newImage;

    invalidate(QRectF(), QGraphicsScene::BackgroundLayer);
}

void CSharedPainterScene::setScaleImageFileItem(std::shared_ptr<CImageFileItem> image,
                                                QGraphicsPixmapItem* pixmapItem) {
    QPixmap pixmap;
    pixmap.loadFromData(image->byteArray());

    // basic size
    int newW = pixmap.width();
    int newH = pixmap.height();
    if (pixmap.width() > DEFAULT_PIXMAP_ITEM_SIZE_W) {
        newW = DEFAULT_PIXMAP_ITEM_SIZE_W;
        newH = (pixmap.height() * DEFAULT_PIXMAP_ITEM_SIZE_W) / pixmap.width();
    }

    newW *= image->scale();
    newH *= image->scale();

    pixmap = pixmap.scaled(newW, newH);
    pixmapItem->setPixmap(pixmap);
}

static QPainterPath createCoveringBorderPath(int borderType, QGraphicsItem* item) {
    QRectF res = item->boundingRect();
    res = item->mapRectToScene(res);

    double left = res.x();
    double top = res.y();
    double w = res.width();
    double h = res.height();

    res.setLeft(left - DEFAULT_COVER_RECT_OFFSET);
    res.setRight(left + w + DEFAULT_COVER_RECT_OFFSET);
    res.setTop(top - DEFAULT_COVER_RECT_OFFSET);
    res.setBottom(top + h + DEFAULT_COVER_RECT_OFFSET);

    QPainterPath path;
    path.addRoundedRect(res, 5, 5);

    return path;
}

void CSharedPainterScene::clearLastItemBorderRect(void) {
    if (lastCoverGraphicsItem_) {
        QGraphicsScene::removeItem(lastCoverGraphicsItem_);
        lastCoverGraphicsItem_ = nullptr;
    }
    timeoutRemoveLastCoverItem_ = 0;
}

void CSharedPainterScene::drawLastItemBorderRect(void) {
    if (!lastAddItem_) return;

    QGraphicsItem* gItem = findById(lastAddItem_->itemId());
    if (!gItem) return;

    // setting style..
    QPainterPath path = createCoveringBorderPath(lastItemBorderType_, gItem);
    if (path.isEmpty()) return;

    clearLastItemBorderRect();

    QAbstractGraphicsShapeItem* lastBorderItem = addPath(path);
    lastBorderItem->setPen(QPen(Util::getComplementaryColor(backgroundColor_, penColor()), 2));
    lastBorderItem->setZValue(currentZValue());
    lastCoverGraphicsItem_ = lastBorderItem;

    // clear
    lastTempBlinkShowFlag_ = true;
    lastTimeValue_ = QDateTime::currentDateTime().toMSecsSinceEpoch();
    timeoutRemoveLastCoverItem_ = DEFAULT_TIMEOUT_REMOVE_LAST_COVER_ITEM;
}

QGraphicsItem* CSharedPainterScene::findById(ItemId id) {
    for (auto gItem : items()) {
        QString itemId = gItem->data(ITEM_DATA_KEY_ITEMID).toString();
        if (itemId.toStdString() == id) {
            return gItem;
        }
    }

    return nullptr;
}

void CSharedPainterScene::startBlinkLastItem() {
    if (!lastAddItem_) return;

    if (lastAddItem_->type() == PT_LINE && lastAddItem_->isMyItem()) {
        // NOTHING TO DO..
    } else {
        clearLastItemBorderRect();

        if (showLastAddItemBorderFlag_) drawLastItemBorderRect();
    }
}

void CSharedPainterScene::commonAddItem(std::shared_ptr<CPaintItem> item,
                                        QGraphicsItem* drawingItem,
                                        int borderType) {
    //  if (!item->isAvailablePosition()) {
    //    DEBUG_LOG_S(L_ERROR) << "position is invalid!";
    //    return;
    //  }
    //  OkGraphicsTextItem* text_item = new OkGraphicsTextItem(drawingItem);
    //  text_item->setTextInteractionFlags(Qt::TextInteractionFlag::TextEditorInteraction);
    //  text_item->  setFlags(QGraphicsItem::ItemIsSelectable  //
    //                      | QGraphicsItem::ItemIsMovable   //
    //                      | QGraphicsItem::ItemIsFocusable //
    //                      | QGraphicsItem::ItemAcceptsInputMethod);
    //  addItem(text_item);

    drawingItem->setPos(item->posX(), item->posY());
    drawingItem->setActive(true);
    drawingItem->setVisible(true);
    drawingItem->setFocus();
    drawingItem->show();
    addItem(drawingItem);

    lastItemBorderType_ = borderType;
    lastAddItem_ = item;

    //
    //  QString tooltip = eventTarget_->onGetToolTipText(item);
    //  if (tooltip.isEmpty() == false)
    //    drawingItem->setToolTip(tooltip);

    clearSelectedItemState();

    // Blink last item feature
    startBlinkLastItem();
}

QRectF CSharedPainterScene::itemBoundingRect(std::shared_ptr<CPaintItem> item) {
    if (!item) return QRectF();

    QGraphicsItem* gItem = findById(item->itemId());
    if (gItem) return gItem->boundingRect();

    return QRectF();
}

void CSharedPainterScene::updateItem(std::shared_ptr<CPaintItem> item) {
    std::lock_guard<std::recursive_mutex> lock(text_mutex_);

    if (!item) return;

    clearLastItemBorderRect();

    QGraphicsItem* graphicsItem = findById(item->itemId());
    if (!graphicsItem) {
        qDebug() << "QGraphicsItem is nullptr!";
        return;
    }

    switch (item->type()) {
        case PaintItemType::PT_TEXT: {
            if (item->isValid()) {
                QString txt = ((CTextItem*)item.get())->text();

                auto tItem = static_cast<OkGraphicsTextItem*>(graphicsItem);
                qDebug() << txt << "instead of:" << tItem->toPlainText();
                if (tItem->toPlainText().compare(txt) != 0) {
                    tItem->setPlainText(txt);
                }
                //                reinterpret_cast<OQGraphicsTextItem*>(graphicsItem)->setPlainText(txt);
            }

            break;
        }
        default:
            break;
    }

    //    if (item->isScalable()) {
    //        if (item->type() == PT_IMAGE_FILE)
    //            setScaleImageFileItem(std::static_pointer_cast<CImageFileItem>(item),
    //            (QGraphicsPixmapItem *) graphicsItem);
    //        else{
    //            graphicsItem->setScale(item->scale());
    //        }
    //    }
}

void CSharedPainterScene::moveItem(std::shared_ptr<CPaintItem> item, double x, double y) {
    if (!item) return;

    qDebug() << " item id:" << qstring(item->itemId()) << " move => pos:{" << x << ", " << y << "}";

    clearLastItemBorderRect();

    QGraphicsItem* gItem = findById(item->itemId());
    if (!gItem) {
        qDebug() << "QGraphicsItem is nullptr!";
        return;
    }

    // freeze move changes notify
    gItem->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    // set new position for this item
    gItem->setPos(x, y);
    // thaw move changes notify
    gItem->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
}

void CSharedPainterScene::drawSendingStatus(std::shared_ptr<CPaintItem> item) {
    if (!item) return;

    //    QGraphicsItem *i = findById(item->itemId());
    //    Q_UNUSED(i);
    // qDebug() << "drawSendingStatus" << item->wroteBytes() <<
    // item->totalBytes();
    // TODO : SENDING PROGRESS
    // How make progress bar and handle it??
    // ....
}

void CSharedPainterScene::drawBackgroundGridLine(int size) {
    gridLineSize_ = size;
    resetBackground(sceneRect());
}

void CSharedPainterScene::drawFile(std::shared_ptr<CFileItem> file) {
    if (!file) {
        return;
    }

    qDebug() << "file:" << file.get();

    //    QIcon icon(file->name());

    //    QPixmap pixmap = icon.pixmap(9999, 9999);
    //    OkGraphicItem<QGraphicsPixmapItem> *item = new
    //    OkGraphicItem<QGraphicsPixmapItem>(this); item->setPixmap(pixmap);
    //    item->setItemData(file);
    //    item->setZValue(ZVALUE_TOPMOST);
    //    if (file->isAvailablePosition())
    //        item->setPos(file->posX(), file->posY());
    //    commonAddItem(file, item, Border_Rect);

    if (ok::base::Files::isImage(file->contentType()))  //
    {
        // 打开图片
        m_networkManager->get(
                file->url(), [this, file](const QByteArray& buffer, const QString& fileName) {
                    Q_UNUSED(fileName)
                    QImage image;
                    if (ok::base::Images::putToImage(buffer, image)) return;

                    std::shared_ptr<CImageItem> item = std::make_shared<CImageItem>(file->itemId());
                    item->setPixmap(QPixmap::fromImage(image));
                    item->setMyItem();
                    this->drawImage(item);
                });
    } else {
        // 打开文件
        emit openFile(file);
    }
}

void CSharedPainterScene::drawImage(std::shared_ptr<CImageItem> image) {
    if (!image) {
        return;
    }

    qDebug() << "image:" << image.get();

    auto item = new OkGraphicItem<QGraphicsPixmapItem>(this);
    item->setPixmap(image->createPixmap());
    item->setItemData(image);
    if (image->isAvailablePosition()) item->setPos(image->posX(), image->posY());
    item->setZValue(currentZValue());

    commonAddItem(image, item, Border_Rect);
}

void CSharedPainterScene::drawImageFile(std::shared_ptr<CImageFileItem> imageFile) {
    auto item = new OkGraphicItem<QGraphicsPixmapItem>(this);

    setScaleImageFileItem(imageFile, item);

    if (imageFile->isAvailablePosition()) item->setPos(imageFile->posX(), imageFile->posY());
    item->setItemData(imageFile);
    item->setZValue(currentZValue());
    commonAddItem(imageFile, item, Border_Rect);
}

void CSharedPainterScene::drawText(std::shared_ptr<CTextItem> text) {
    qDebug() << "item id:" << qstring(text->itemId()) << "text:" << text->text();

    clearSelectedItemState();

    auto item = new OkGraphicItem<OkGraphicsTextItem>(this);
    item->setFont(text->font());
    item->setPlainText(text->text());
    item->setDefaultTextColor(text->color());
    item->setZValue(currentZValue());
    item->setItemData(text);

    connect(item, &OkGraphicsTextItem::itemChanged, this, &CSharedPainterScene::onItemChanged);

    connect(item, SIGNAL(lostFocus(OkQGraphicsTextItem*)), this,
            SLOT(editorLostFocus(OkQGraphicsTextItem*)));

    // 结束原有文本元素。
    //     endOfDrawText();

    commonAddItem(text, item, Border_Rect);
}

void CSharedPainterScene::drawLine(std::shared_ptr<CLineItem> line_ptr) {
    CLineItem* line = line_ptr.get();

    if (line->pointCount() <= 0) return;

    QPainterPath painterPath;
    painterPath.moveTo(*line->point(0));

    if (line->pointCount() > 1) {
        for (size_t i = 1; i < line->pointCount(); i++) {
            painterPath.lineTo(*line->point(i));
        }

        auto item = new OkGraphicItem<QGraphicsPathItem>(this);

        if (line->isAvailablePosition()) item->setPos(line->posX(), line->posY());

        item->setPath(painterPath);
        item->setZValue(currentZValue());
        item->setPen(
                QPen(line->color(), line->width(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        item->setItemData(line_ptr);
        item->setFlags(QGraphicsItem::ItemIsSelectable   //
                       | QGraphicsItem::ItemIsMovable    //
                       | QGraphicsItem::ItemIsFocusable  //
                       | QGraphicsItem::ItemIsPanel);
        commonAddItem(line_ptr, item, Border_PainterPath);
    } else {
        QPointF to = *line->point(0);

        double x = to.x() - (line->width() / 2);
        double y = to.y() - (line->width() / 2);
        QRectF rect(x, y, line->width(), line->width());

        auto item = new OkGraphicItem<QGraphicsEllipseItem>(this);
        if (line->isAvailablePosition()) item->setPos(line->posX(), line->posY());
        item->setRect(rect);
        item->setPen(QPen(line->color(), 1));
        item->setBrush(QBrush(line->color()));
        item->setZValue(currentZValue());
        item->setItemData(line_ptr);
        commonAddItem(line_ptr, item, Border_Ellipse);
    }
}

void CSharedPainterScene::setBackgroundColor(int r, int g, int b, int a) {
    backgroundColor_ = QColor(r, g, b, a);
    resetBackground(sceneRect());
}

void CSharedPainterScene::clearBackgroundImage(void) {
    backgroundImageItem_ = std::shared_ptr<CBackgroundImageItem>();
    backgroundPixmap_ = QPixmap();
    resetBackground(sceneRect());
}

void CSharedPainterScene::drawBackgroundImage(std::shared_ptr<CBackgroundImageItem> image) {
    backgroundImageItem_ = image;

    if (image) {
        backgroundPixmap_ = image->createPixmap();

        int newSceneW = sceneRect().width();
        int newSceneH = sceneRect().height();

        QSize size = backgroundPixmap_.size();
        if (newSceneW < size.width()) newSceneW = size.width();
        if (newSceneH < size.height()) newSceneH = size.height();
        setSceneRect(0, 0, newSceneW, newSceneH);
    } else {
        clearBackgroundImage();
        return;
    }

    resetBackground(sceneRect());
}

void CSharedPainterScene::drawLineStart(const QPointF& pt, const QColor& clr, int width) {
    double x = pt.x() - (double(width) / 2.f);
    double y = pt.y() - (double(width) / 2.f);
    QRectF rect(x, y, width, width);

    QGraphicsEllipseItem* item = addEllipse(rect, QPen(clr, 1), QBrush(clr));
    item->setZValue(currentZValue());

    tempLineItemList_.push_back(item);
}

void CSharedPainterScene::drawLineTo(const QPointF& pt1, const QPointF& pt2, const QColor& clr,
                                     int width) {
    QGraphicsLineItem* item = addLine(pt1.x(), pt1.y(), pt2.x(), pt2.y(),
                                      QPen(clr, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    item->setZValue(currentLineZValue_);
    tempLineItemList_.push_back(item);
}

void CSharedPainterScene::resizeImage(QImage* image, const QSize& newSize) {
    if (image->size() == newSize) return;

    QImage newImage(newSize, QImage::Format_RGB32);
    newImage.fill(qRgb(255, 255, 255));

    QPainter painter(&newImage);
    painter.drawImage(QPoint(0, 0), *image);
    *image = newImage;
}

void CSharedPainterScene::addImageFileItem(const QPointF& pos, const QString& path) {
    std::shared_ptr<CImageFileItem> imageItem =
            std::make_shared<CImageFileItem>(CreateItemId(), path);

    imageItem->setMyItem();
    imageItem->setPos(pos.x(), pos.y());

    fireEvent_DrawItem(imageItem);
}

void CSharedPainterScene::addGeneralFileItem(const QPointF& pos, const QString& path) {
    std::shared_ptr<CFileItem> fileItem = std::make_shared<CFileItem>(CreateItemId(), (path));

    fileItem->setMyItem();
    fileItem->setPos(pos.x(), pos.y());

    fireEvent_DrawItem(fileItem);
}

void CSharedPainterScene::addGeneralUrlItem(const QPointF& pos,
                                            const lib::backend::FileResult& urlInfo) {
    std::shared_ptr<CFileItem> fileItem = std::make_shared<CFileItem>(CreateItemId(), urlInfo.name);

    fileItem->setUrl(urlInfo.url);
    fileItem->setContentType(urlInfo.contentType);
    fileItem->setMyItem();
    fileItem->setPos(pos.x(), pos.y());
    fireEvent_DrawItem(fileItem);
}

void CSharedPainterScene::drawBackground(QPainter* painter, const QRectF& rect) {
    painter->drawImage(rect, image_, rect);
}

void CSharedPainterScene::doLowQualityMoveItems(void) {
    if (tempMovingItemList_.size() <= 0) return;

    ITEM_SET::iterator it = tempMovingItemList_.begin();
    for (; it != tempMovingItemList_.end(); it++) {
        //        std::shared_ptr<CPaintItem> item = *it;
        eventTarget_->onMoveItem(*it);
    }

    tempMovingItemList_.clear();
}

void CSharedPainterScene::onItemMoving(std::shared_ptr<CPaintItem> item, const QPointF& newPos) {
    //        qDebug() << "item id:" << qstring(item->itemId())
    //                            << "newPos:{" << newPos.x() << ", " <<
    //                            newPos.y() << "}";

    item->setPos(newPos.x(), newPos.y());

    if (hiqhQualityMoveItemMode_) {
        eventTarget_->onMoveItem(item);
    } else {
        tempMovingItemList_.insert(item);
    }
}

void CSharedPainterScene::onItemUpdate(std::shared_ptr<CPaintItem> item) {
    if (!item) {
        return;
    }

    eventTarget_->onUpdateItem(item);
}

void CSharedPainterScene::onItemClipboardCopy(std::shared_ptr<CPaintItem> item) {
    if (!item) {
        return;
    }
    item->copyToClipboard();
}

void CSharedPainterScene::onItemRemove(std::shared_ptr<CPaintItem> item) {
    eventTarget_->onRemoveItem(item);
}

void CSharedPainterScene::keyPressEvent(QKeyEvent* event) {
    qDebug() << "keyPressEvent" << event;
    if (event->key() == Qt::Key_A && event->modifiers() == Qt::ControlModifier) {
        // CTRL+A
        QList<QGraphicsItem*> list = items();
        for (int i = 0; i < list.size(); ++i) {
            list.at(i)->setSelected(true);
        }
    } else if (event->key() == Qt::Key_C && event->modifiers() == Qt::ControlModifier) {
        // CTRL+C
        QList<QGraphicsItem*> list = selectedItems();
        for (int i = 0; i < list.size(); ++i) {
            std::shared_ptr<CPaintItem> paintItem = findPaintItem(list.at(i));
            if (paintItem) {
                paintItem->copyToClipboard(i == 0 ? true : false);
            }
        }
    } else if (event->key() == Qt::Key_V && event->modifiers() == Qt::ControlModifier) {
        // CTRL+V
        onClipboardPaste();
    } else if (event->key() == Qt::Key_Delete) {
        //        DELETE
        removeSelections();
    } else if (event->key() == Qt::Key_A && event->modifiers() == Qt::AltModifier) {
    }
    QGraphicsScene::keyPressEvent(event);
}

void CSharedPainterScene::keyReleaseEvent(QKeyEvent* event) {
    QGraphicsScene::keyReleaseEvent(event);
}

void CSharedPainterScene::dragEnterEvent(QGraphicsSceneDragDropEvent* e) {
    qDebug() << e;

    int cnt = 0;
    const QMimeData* mimeData = e->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        for (int i = 0; i < urlList.size(); ++i) {
            QString path = urlList.at(i).toLocalFile();
            if (!QFileInfo(path).isFile()) continue;
            cnt++;
        }
    }

    if (cnt > 0) {
        e->acceptProposedAction();
    } else {
        e->ignore();
    }
}

void CSharedPainterScene::dragLeaveEvent(QGraphicsSceneDragDropEvent* e) {
    qDebug() << e;
    emit dragLeaveEvent();
    e->accept();
}

void CSharedPainterScene::dragMoveEvent(QGraphicsSceneDragDropEvent* e) {
    //    DEBUG_LOG(("...")) ;
    e->acceptProposedAction();
}

void CSharedPainterScene::dropEvent(QGraphicsSceneDragDropEvent* e) {
    qDebug() << e;

    emit dropEvent();

    const QMimeData* mimeData = e->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        for (int i = 0; i < urlList.size(); ++i) {
            QString path = urlList.at(i).toLocalFile();

            if (!QFileInfo(path).isFile()) continue;

            emit receivedDropFile(path);

            //            QFile* file = new QFile(path);
            //            nm_->GetURL(file, [this](QJsonObject& result){
            //                //清除文件
            ////                delete file;

            //                backend::FileResult _dr =
            //                network::NetworkManager::parseDownloadResult(result);
            //                if(!_dr.success){
            //                     DEBUG_LOG_S(L_ERROR) << "Upload file error!";
            //                }

            //                emit openUrl(_dr);

            //            });

            //            // TODO : Image check, is this the fastest way to check it?
            //            bool isImageFile = (QImage(path).isNull() == false);
            //            DEBUG_LOG(("isImageFile:%1").arg(isImageFile));

            addGeneralFileItem(e->scenePos(), path);

            //            if (isImageFile)
            //                addImageFileItem(e->scenePos(), path);
            //            else{
            //                QString file = mimeData->text();
            //                DEBUG_LOG(("File:%1").arg(file));

            // PDF, DOCX, PPTX, XLSX
            //                addGeneralFileItem(e->scenePos(), path);

            // 发送文件接收事件
            //                 emit receivedDropFile(path);

            // 上传文件
            //                 if(parent_){

            //                }

            //                std::shared_ptr<CTextItem> textItem = createTextItem(s);
            //                fireEvent_DrawItem(textItem);

            //            }
        }
    }
}

void CSharedPainterScene::mousePressEvent(QGraphicsSceneMouseEvent* evt) {
    qDebug() << "mousePressEvent" << evt;
    if (_disabled) {
        return;
    }

    prevPos_ = evt->scenePos();
    qDebug() << "pos" << prevPos_;

    switch (_toolboxType) {
        case painter::ToolboxType::P_MOVE: {
            QGraphicsScene::mousePressEvent(evt);
            break;
        }
        case painter::ToolboxType::P_PEN: {
            drawFlag_ = true;

            currLineItem_ = std::make_shared<CLineItem>(CreateItemId(), penClr_, penWidth_);

            currLineItem_->addPoint(prevPos_);
            currLineItem_->setMyItem();

            currentLineZValue_ = currentZValue();

            drawLineStart(prevPos_, currLineItem_->color(), currLineItem_->width());

            break;
        }
        case ToolboxType::P_TEXT: {
            currTextItem_ = createTextItem("");
            fireEvent_DrawItem(currTextItem_);
            break;
        }

        default:
            break;
    }
}

void CSharedPainterScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* evt) {
    qDebug() << "mouseReleaseEvent" << evt;

    drawFlag_ = false;

    switch (_toolboxType) {
        case ToolboxType::P_PEN: {
            fireEvent_DrawItem(currLineItem_);

            //        currLineItem_ = std::make_shared<CLineItem>();

            for (size_t i = 0; i < tempLineItemList_.size(); i++) {
                QGraphicsScene::removeItem(tempLineItemList_[i]);
            }
            tempLineItemList_.clear();

            resetBackground(sceneRect());
            break;
        }
        case ToolboxType::P_TEXT: {
            break;
        }
        default: {
            doLowQualityMoveItems();
            break;
        }
    }

    QGraphicsScene::mouseReleaseEvent(evt);
}

/**
 * Mouse move event
 * @brief CSharedPainterScene::mouseMoveEvent
 * @param evt
 */
void CSharedPainterScene::mouseMoveEvent(QGraphicsSceneMouseEvent* evt) {
    if (_toolboxType == painter::ToolboxType::P_PEN) {
        if (!drawFlag_) return;

        QPointF to = evt->scenePos();
        //    to.setX(to.x() + CURSOR_OFFSET_X);
        //    to.setY(to.y() + CURSOR_OFFSET_Y);

        if (currLineItem_) {
            drawLineTo(prevPos_, to, currLineItem_->color(), currLineItem_->width());
            currLineItem_->addPoint(to);
        }

        prevPos_ = to;
    } else {
        QGraphicsScene::mouseMoveEvent(evt);
    }
}

std::shared_ptr<CTextItem> CSharedPainterScene::createTextItem(const QString& s) {
    QFont font;
    font.setPixelSize(textWidth_);
    std::shared_ptr<CTextItem> textItem =
            std::make_shared<CTextItem>(CreateItemId(), s, font, textClr_);
    textItem->setPos(prevPos_.x(), prevPos_.y());
    return textItem;
}

void CSharedPainterScene::editorLostFocus(OkGraphicsTextItem* item) {
    qDebug() << "item:" << item;
    std::lock_guard<std::recursive_mutex> lock(text_mutex_);

    delayCaller_->call(200, [item, this]() {
        QTextCursor cursor = item->textCursor();
        cursor.clearSelection();
        item->setTextCursor(cursor);

        if (item->toPlainText().isEmpty()) {
            this->removeItem(item);
            item->deleteLater();
        }
    });
}

void CSharedPainterScene::onItemChanged(OkGraphicsTextItem* item,                      //
                                        QGraphicsTextItem::GraphicsItemChange change,  //
                                        const QVariant& value) {
    qDebug() << "change:" << change << value;
    auto textItem = qgraphicsitem_cast<OkGraphicsTextItem*>(item);
    if (change == QGraphicsTextItem::GraphicsItemChange::ItemFlagsHaveChanged) {
        qDebug() << "text:" << textItem->toPlainText();

        std::shared_ptr<CPaintItem> pItem = findPaintItem(item);
        if (pItem) {
            CTextItem* tItem = static_cast<CTextItem*>(pItem.get());
            tItem->setText(textItem->toPlainText());
        }

        onItemUpdate(pItem);
    }

    //        QFont font = textItem->font();
    //        fontCombo->setCurrentFont(font);
    //        fontSizeCombo->setEditText(QString().setNum(font.pointSize()));
    //        boldAction->setChecked(font.weight() == QFont::Bold);
    //        italicAction->setChecked(font.italic());
    //        underlineAction->setChecked(font.underline());
}

void CSharedPainterScene::onScreenCapture(QPixmap pixmap) {
    qDebug() << pixmap;
    emit receivedScreenCapture(pixmap);

    //    std::shared_ptr<CImageItem> item = std::make_shared<CImageItem>();

    //    item->setPixmap(pixmap);
    //    item->setPos(20, 20);

    //    if(!temp_file_.isOpen()){
    //        temp_file_.remove();
    //    }

    //    temp_file_.setAutoRemove(false);
    //    temp_file_.open();

    //    if(temp_file_.isOpen()){
    //        QString filename = temp_file_.fileName();
    //        qDebug() << "filename: "<< filename;

    //        QByteArray ba;
    //        QBuffer buffer(&ba);
    //        buffer.open(QIODevice::WriteOnly);
    //        pixmap.save(&buffer, "PNG");

    //        temp_file_.write(ba);
    //        temp_file_.close();
    //    }

    //    QBuffer buffer(&m_baThumb);
    //        buffer.open(QIODevice::WriteOnly);
    //        pixmap.save(&buffer, "PNG");

    //    std::shared_ptr<CFileItem> file = std::make_shared<CFileItem>();

    //    this->draw(file);
}

void CSharedPainterScene::onClipboardPaste() {
    const QClipboard* clipboard = QGuiApplication::clipboard();
    const QMimeData* mimeData = clipboard->mimeData();

    if (mimeData->hasImage()) {
        //        int w = 0;
        //        int h = 0;

        std::shared_ptr<CImageItem> item = std::make_shared<CImageItem>(CreateItemId());
        QPixmap pixmap(qvariant_cast<QPixmap>(mimeData->imageData()));
        item->setPixmap(pixmap);
        item->setMyItem();

        //        int w = pixmap.width();
        //        int h = pixmap.height();

        //        DEBUG_LOG(("Image w:%1 h:%2").arg(w).arg(h)) ;

        /*QPointF pos = Util::calculateNewItemPos(sceneRect().width(),
           ceneRect().height(), parent->mapFromGlobal(QCursor::pos()).x(),
                    parent->mapFromGlobal(QCursor::pos()).y(),
                    w, h);*/

        //        item->setPos(20, 20);

        drawImage(item);
    } else if (mimeData->hasText()) {
        // QFont f(tr("Gulim"));
        // f.setPixelSize(10);
        // addTextItem(mimeData->text(), f,
        // Util::getComplementaryColor(canvas_->backgroundColor()));

        QString s = mimeData->text();

        std::shared_ptr<CTextItem> textItem = createTextItem(s);

        fireEvent_DrawItem(textItem);
    }
}

void CSharedPainterScene::onCtrollerChecked(WB_CTRL ctrl, bool isChecked) {
    if (ctrl == WB_CTRL::WB) {
        _disabled = isChecked;
        if (_disabled) {
            setCursor(Qt::ForbiddenCursor);
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }
}

void CSharedPainterScene::removeSelections() {
    qDebug() << "removeSelections";
    for (auto item : selectedItems()) {
        removeItem(item);
    }
}

void CSharedPainterScene::removeItem(QGraphicsItem* item) {
    qDebug() << "remove" << item;
    if (!item) return;

    std::lock_guard<std::recursive_mutex> lock(item_mutex_);

    QGraphicsScene::removeItem(item);

    std::shared_ptr<CPaintItem> paintItem = findPaintItem(item);
    if (paintItem) {
        eventTarget_->onRemoveItem(paintItem);
    }
}

void CSharedPainterScene::removePaintItem(CPaintItem* item) {
    if (!item) return;

    std::lock_guard<std::recursive_mutex> lock(item_mutex_);

    qDebug() << "remove PaintItem id:" << qstring(item->itemId());

    commandMgr_->removeItem(item);

    //    if (lastAddItem_.get() == item)
    //        clearLastItemBorderRect();

    QGraphicsItem* graphicsItem = findById(item->itemId());
    if (graphicsItem) {
        QGraphicsScene::removeItem(graphicsItem);
    }
}

void CSharedPainterScene::removeInvalidItems() {
    std::lock_guard<std::recursive_mutex> lock(item_mutex_);

    QList<QGraphicsItem*> list = items();
    for (int i = 0; i < list.size(); ++i) {
        QGraphicsItem* item = list.at(i);
        std::shared_ptr<CPaintItem> paintItem = findPaintItem(item);
        if (paintItem && !paintItem->isValid()) {
            removePaintItem(paintItem.get());
        }
    }
}

}  // namespace module::painter
