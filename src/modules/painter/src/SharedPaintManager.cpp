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

#include "SharedPaintManager.h"
#include "PainterWidgetProxy.h"
#include "SharedPaintCommandManager.h"
#include "painterevent.h"

#include <QBuffer>
#include "lib/board/smartboarddraw.h"
#include "lib/board/smartboarddrawline.h"
#include "lib/board/smartboarddrawmove.h"
#include "lib/board/smartboarddrawremove.h"
#include "lib/board/smartboarddrawtext.h"

#include "base/times.h"
#include "src/base/logs.h"
#include "src/base/utils.h"

#define START_SERVER_PORT 4001
#define DEFAULT_BROADCAST_PORT 3336
#define DEFAULT_BROADCAST_UDP_PORT_FOR_TEXTMSG 3338
#define START_UDP_LISTEN_PORT 5001
#define START_UDP_LISTEN_PORT_FOR_STREAM 6000

#define TIMEOUT_SYNC_MSEC 5000

namespace module::painter {

static int margin = 5;

class ISharedPaintEvent;

CSharedPaintManager::CSharedPaintManager(QWidget* painter)
        : painter_(painter)
        , enabled_(true)
        , syncStartedFlag_(false)
        , commandMngr_(nullptr)
        , listenTcpPort_(-1)
        , listenUdpPort_(-1)
        , retryServerReconnectCount_(0)
        , lastConnectMode_(INIT_MODE)
        , lastConnectPort_(-1)
        , findingServerMode_(false)
        , lastWindowWidth_(0)
        , lastWindowHeight_(0)
        , lastCanvasWidth_(0)
        , lastCanvasHeight_(0)
        , lastScrollHPos_(-1)
        , lastScrollVPos_(-1)
        , gridLineSize_(0)
        , lastPacketId_(-1)
        , backgroundColor_(Qt::white)
        , m_networkManager(std::make_unique<lib::network::NetworkHttp>(this)) {
    qDebug() << "begin";

    // 命令管理器
    commandMngr_ = std::make_shared<CSharedPaintCommandManager>(this);

    // 画板事件
    painterEvent_ = std::make_shared<PainterEvent>();

    QRect rect =
            QRect(margin, margin, painter_->width() - margin * 2, painter_->height() - margin * 2);

    // CSharedPainterScene
    _scene = std::make_shared<CSharedPainterScene>(painterEvent_.get());
    _scene->setCommandMgr(commandMngr_);
    _scene->setSceneRect(rect);
    //    _scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    //    _scene->setStickyFocus(true);

    auto palette = _scene->palette();
    palette.setColor(QPalette::Window, QColor(0, 0, 0, 100));
    _scene->setPalette(palette);

    // QGraphicsView

    _view = std::make_unique<QGraphicsView>(_scene.get(), painter_);
    _view->setRenderHint(QPainter::Antialiasing);
    _view->setCacheMode(QGraphicsView::CacheBackground);
    _view->setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);
    _view->setDragMode(QGraphicsView::ScrollHandDrag);
    _view->setFocusPolicy(Qt::ClickFocus);

    //_view->setAutoFillBackground(true);
    //    _view->setAttribute(Qt::WA_TranslucentBackground, true);
    //    _view->setStyleSheet("border:1px solid red");

    _view->show();

    // 画板渲染器
    painterRenderer_ = std::make_shared<PainterRenderer>();
    painterRenderer_->setSharedPainterScene(_scene.get());

    myUserInfo_ = std::shared_ptr<CPaintUser>(new CPaintUser(true));

    // 画板数据分发
    painterDispatcher_ = std::make_shared<PainterDispatcher>();

    registerObserver(painterDispatcher_.get());
    registerObserver(painterRenderer_.get());

    painterEvent_->setManager(this);

    // 接收事件
    connect(_scene.get(), SIGNAL(receivedDropFile(QString)), this,
            SLOT(onReceivedDropFile(QString)));

    connect(_scene.get(), SIGNAL(receivedScreenCapture(QPixmap)), this,
            SLOT(onReceivedScreenCapture(QPixmap)));

    qDebug() << "end";
}

CSharedPaintManager::~CSharedPaintManager(void) {
    stopServer();
    close();
    stopListenBroadCast();
}

void CSharedPaintManager::initialize(const std::string& myId) {
    struct SPaintUserInfoData data;
    data.userId = myId;
    myUserInfo_->setData(data);
}

void CSharedPaintManager::openDialog() {}

void CSharedPaintManager::setToolType(ToolboxType toolboxType) {
    _scene->setToolType(toolboxType);
}

void CSharedPaintManager::setSize(const QSize& size) {
    _view->resize(size);

    QRect rect =
            QRect(margin, margin, painter_->width() - margin * 2, painter_->height() - margin * 2);
    _scene->setSceneRect(rect);
}

void CSharedPaintManager::onTimeoutSyncStart(void) {
    if (syncStartedFlag_ == false) close();
}

void CSharedPaintManager::onDraw(lib::board::SmartBoardDraw* draw) {
    if (!draw) {
        return;
    }

    qDebug() << "id:" << qstring(draw->id());

    const lib::board::DrawLine* line =
            draw->findPlugin<lib::board::DrawLine>(lib::board::DrawType::Line);
    if (line) {
        doLine(draw, line);
        return;
    }

    const lib::board::DrawMove* move =
            draw->findPlugin<lib::board::DrawMove>(lib::board::DrawType::Move);
    if (move) {
        doMove(draw, move);
        return;
    }

    const lib::board::DrawRemove* remove =
            draw->findPlugin<lib::board::DrawRemove>(lib::board::DrawType::Remove);
    if (remove) {
        doRemove(draw, remove);
    }

    const lib::board::DrawFile* file =
            draw->findPlugin<lib::board::DrawFile>(lib::board::DrawType::File);
    if (file) {
        doFile(draw, file);
        return;
    }

    const lib::board::DrawText* text =
            draw->findPlugin<lib::board::DrawText>(lib::board::DrawType::Text);
    if (text) {
        doText(draw, text);
        return;
    }
}

void CSharedPaintManager::doText(lib::board::SmartBoardDraw* draw,
                                 const lib::board::DrawText* text) {
    qDebug() << "text:" << text;

    std::shared_ptr<CTextItem> _item = std::make_shared<CTextItem>("");
    _item->deserialize(std::shared_ptr<lib::board::SmartBoardDraw>(draw));

    CPaintItem::Action act = _item->action();
    switch (act) {
        case CPaintItem::Action::Create: {
            auto y = commandMngr_->addHistoryItem(_item);
            if (y) {
                _item->setCanvas(_scene.get());
                _item->draw();
            }
            break;
        }
        case CPaintItem::Action::Update: {
            std::shared_ptr<CPaintItem> _uItem = commandMngr_->findItem("", _item->itemId());
            if (_uItem) {
                std::static_pointer_cast<CTextItem>(_uItem)->setText(_item->text());
                _uItem->update();
            }
            break;
        }
    }
}

void CSharedPaintManager::doFile(lib::board::SmartBoardDraw* draw,
                                 const lib::board::DrawFile* file) {
    qDebug() << "file:" << file;

    std::shared_ptr<CFileItem> _item = std::make_shared<CFileItem>("");
    _item->deserialize(std::shared_ptr<lib::board::SmartBoardDraw>(draw));

    auto y = commandMngr_->addHistoryItem(_item);
    if (!y) return;

    _item->setCanvas(_scene.get());
    _item->draw();
}

void CSharedPaintManager::doMove(lib::board::SmartBoardDraw* draw,
                                 const lib::board::DrawMove* move) {
    qDebug() << "move:" << move;

    std::shared_ptr<CPaintItem> _item = commandMngr_->findItem("", move->moveId());
    if (!_item) {
        qWarning() << "moveId:" << qstring(move->moveId()) << " is not exist!";
        return;
    }

    _item->move(move->position().x, move->position().y);
}

void CSharedPaintManager::doRemove(lib::board::SmartBoardDraw* draw,
                                   const lib::board::DrawRemove* remove) {
    qDebug() << "remove:" << remove;

    std::shared_ptr<CPaintItem> _item = commandMngr_->findItem("", remove->removeId());
    if (!_item) {
        qWarning() << "remove:" << qstring(remove->removeId()) << " is not exist!";
        return;
    }

    _scene->removePaintItem(_item.get());
}

void CSharedPaintManager::doLine(lib::board::SmartBoardDraw* draw,
                                 const lib::board::DrawLine* line) {
    qDebug() << "line:" << line;
    std::shared_ptr<CLineItem> _item = std::make_shared<CLineItem>("");
    _item->deserialize(std::shared_ptr<lib::board::SmartBoardDraw>(draw));

    auto y = commandMngr_->addHistoryItem(_item);
    if (!y) return;

    _item->setCanvas(_scene.get());
    _item->draw();
}

bool CSharedPaintManager::addPaintItem(std::shared_ptr<CPaintItem> item) {
    if (!enabled_) return false;

    item->setOwner("");
    auto y = commandMngr_->addHistoryItem(item);
    if (y) {
        std::shared_ptr<CAddItemCommand> command = std::make_shared<CAddItemCommand>(item);
        y = commandMngr_->executeCommand(command);
    }

    return y;
}

bool CSharedPaintManager::startListenBroadCast(void) {
    return true;
}

bool CSharedPaintManager::startFindingServer(void) {
    stopListenBroadCast();
    _stopFindingServer();
    return true;
}

bool CSharedPaintManager::startServer(int port) {
    return true;
}

void CSharedPaintManager::stopListenBroadCast(void) {
    qDebug() << "stopListenBroadCast";
}

void CSharedPaintManager::_stopFindingServer(void) {
    qDebug() << "_stopFindingServer" << findingServerMode_;

    if (!findingServerMode_) return;

    findingServerMode_ = false;
}

void CSharedPaintManager::stopFindingServer(void) {}

void CSharedPaintManager::stopServer(void) {}

bool CSharedPaintManager::deserializeData(const char* data, size_t size) {
    return true;
}

std::string CSharedPaintManager::serializeData(const std::string* target) {
    std::string allData;

    return allData;
}

void CSharedPaintManager::clearAllItems()
// this function must be called on main thread!
{
    qDebug() << "CSharedPaintManager::clearAllItems()";

    backgroundColor_ = Qt::white;
    backgroundImageItem_ = std::shared_ptr<CBackgroundImageItem>();

    _scene->clearScreen();

    // all data clear
    if (commandMngr_) commandMngr_->clear();

    // history must be removed with removing all paint item.
    //		clearAllHistoryUsers();
}
void CSharedPaintManager::toDrawQueue(lib::board::SmartBoardDraw* draw) {
    onDraw(draw);
}

void CSharedPaintManager::onReceivedDropFile(QString path) {
    qDebug() << path;

    QFile* file = new QFile(path);
    if (!file || !file->exists()) {
        qWarning() << "file is not exist!";
        return;
    }

    //        m_networkManager->upload(file, [this](QJsonObject &result)
    //                                {
    //        //解析返回的数据
    //        backend::FileResult _dr = network::NetworkHttp::parseDownloadResult(result);
    //        if(!_dr.success){
    //            DEBUG_LOG_S(L_ERROR) << "Upload error!";
    //        }
    //        //添加到视图
    //        _scene->addGeneralUrlItem(QPointF(0, 0), _dr); });
}

/** Receive screen capture image
 * @brief CSharedPaintManager::onReceivedScreenCapture
 * @param pixmap
 */
void CSharedPaintManager::onReceivedScreenCapture(QPixmap pixmap) {
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG");

    //    QString timestamp = ok::base::Times::GetTimestamp();
    //    QString filename = qsl("ScreenCapture_%1").arg(timestamp);
    //    QString contentType = "image/png";

    //        network::ByteInfo byteInfo = {ba, filename, contentType};
    //        m_networkManager->GetURL(byteInfo, [this](QJsonObject &result)
    //     {
    //        //解析返回的数据
    //        backend::FileResult _dr = network::NetworkHttp::parseDownloadResult(result);
    //        if(!_dr.success){
    //            DEBUG_LOG_S(L_ERROR) << "Upload error!";
    //        }
    // 添加到视图
    //        _scene->addGeneralUrlItem(QPointF(0, 0), _dr); });
}

}  // namespace module::painter
