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

#include <list>
#include <map>

#ifdef Q_OS_WIN
#undef DrawText  // 避免windows宏冲突
#endif

#include "lib/board/smartboarddraw.h"
#include "lib/board/smartboarddrawline.h"
#include "lib/board/smartboarddrawmove.h"
#include "lib/board/smartboarddrawremove.h"
#include "lib/board/smartboarddrawtext.h"
#include "lib/network/NetworkHttp.h"
#include "src/base/logs.h"

#include "DefferedCaller.h"
#include "PaintItem.h"
#include "SharedPaintPolicy.h"

#include "PaintUser.h"
#include "SharedPaintCommandManager.h"

#include "Base.h"
#include "SharedPainterScene.h"
#include "isharedpaintevent.h"
#include "lib/backend/BaseService.h"

namespace SmartBoard {
class DrawText;
}

namespace module::painter {

class CAddItemTask;
class CRemoveItemTask;
class CUpdateItemTask;
class CMoveItemTask;
class PainterRenderer;
class PainterEvent;
class PainterDispatcher;

using namespace gloox;

class CSharedPaintManager : public QObject {
    Q_OBJECT

    typedef std::map<std::string, std::shared_ptr<CSharedPaintItemList>> ITEM_LIST_MAP;
    //	typedef std::map< std::string, std::shared_ptr<CPaintUser> > USER_MAP;
    //	typedef std::vector< std::shared_ptr<CPaintSession> > SESSION_LIST;
    //	typedef std::map< std::string, std::shared_ptr<CNetUdpSession> >
    // UDP_SESSION_MAP;

public:
    CSharedPaintManager(QWidget* painter);
    ~CSharedPaintManager(void);

    //    static CSharedPaintManager* Get();

    void initialize(const std::string& myId);  // TODO : initialize check

    void openDialog();

    void setToolType(ToolboxType toolboxType);

    void setSize(const QSize& size);

    const std::string& myId(void) {
        return myUserInfo_->userId();
    }

    void closeSession(void) {
        setEnabled(true);

        //		clearAllUsers();
        //		clearAllSessions();
        //		stopFindingServer();

        syncStartedFlag_ = false;
        findingServerMode_ = false;
    }

    void close(void) {
        qDebug() << "CSharedPaintManager::close()";

        clearScreen(false);  // DO NOT NOTIFY TO OTHERS.. JUST TRIGGER CLEAR SCREEN EVENT
        clearAllItems();

        closeSession();
    }

    void registerObserver(ISharedPaintEvent* obs) {
        observers_.remove(obs);
        observers_.push_back(obs);
    }

    void unregisterObserver(ISharedPaintEvent* obs) {
        observers_.remove(obs);
    }

    // Network
public:
    bool startServer(int port = 0);

    bool startFindingServer(void);

    void stopFindingServer(void);

    void setPaintChannel(const std::string& channel);

    void changeNickName(const std::string& nickName);

    bool addPaintItem(std::shared_ptr<CPaintItem> item);

    void doLine(lib::board::SmartBoardDraw* draw, const lib::board::DrawLine* line);

    void doText(lib::board::SmartBoardDraw* draw, const lib::board::DrawText* text);

    void doFile(lib::board::SmartBoardDraw* draw, const lib::board::DrawFile* file);

    void doMove(lib::board::SmartBoardDraw* draw, const lib::board::DrawMove* move);

    void doRemove(lib::board::SmartBoardDraw* draw, const lib::board::DrawRemove* remove);

    std::shared_ptr<CSharedPainterScene> scene() {
        return _scene;
    }
    // Shared Paint Action
public:
    void setEnabled(bool enabled) {
        enabled_ = enabled;
    }

    void redoCommand(void) {
        if (!enabled_) return;

        commandMngr_->redoCommand();
    }

    void undoCommand(void) {
        if (!enabled_) return;

        commandMngr_->undoCommand();
    }

    bool deserializeData(const char* data,
                         size_t size);  // TODO throw exception logic

    std::string serializeData(const std::string* target = nullptr);

    std::shared_ptr<CPaintItem> findPaintItem(const std::string& owner, ItemId itemId) {
        return commandMngr_->findItem(owner, itemId);
    }

    void updatePaintItem(std::shared_ptr<CPaintItem> item) {
        if (!enabled_) return;

        std::shared_ptr<CUpdateItemCommand> command =
                std::shared_ptr<CUpdateItemCommand>(new CUpdateItemCommand(item));
        commandMngr_->executeCommand(command);
    }

    void movePaintItem(std::shared_ptr<CPaintItem> item) {
        if (!enabled_) return;

        std::shared_ptr<CMoveItemCommand> command =
                std::shared_ptr<CMoveItemCommand>(new CMoveItemCommand(item));
        commandMngr_->executeCommand(command);
    }

    void removePaintItem(std::shared_ptr<CPaintItem> item) {
        if (!enabled_) return;

        std::shared_ptr<CRemoveItemCommand> command =
                std::shared_ptr<CRemoveItemCommand>(new CRemoveItemCommand(item));
        commandMngr_->executeCommand(command);
    }

    void clearScreen(bool sendData = true) {
        caller_.performMainThread(std::bind(&CSharedPaintManager::fireObserver_ClearScreen, this));
    }

    //        void notifyChangeCanvasScrollPos(int scrollH, int scrollV) {

    //        }

    //        void notifyResizingCanvas(int width, int height) {

    //        }

    //        void notifyResizingMainWindow(int width, int height) {

    //        }

    //        void notifyResizingWindowSplitter(const std::vector<int> &sizes) {

    //        }

private:
    void clearAllItems(void);

    // Playback
public:
    bool isPlaybackMode(void) {
        return commandMngr_->isPlaybackMode();
    }

    size_t historyTaskCount(void) {
        return commandMngr_->historyTaskCount();
    }

    void plabackTo(int position) {
        commandMngr_->playbackTo(position);
    }

    void setAllowPainterToDraw(const std::string& userId, bool enabled) {
        commandMngr_->setAllowPainterToDraw(userId, enabled);
    }

    bool isAllowPainterToDraw(const std::string& userId) {
        return commandMngr_->isAllowPainterToDraw(userId);
    }

private:
    //	void sendAllSyncData( int toSessionId )
    //	{
    //		if( isAlwaysP2PMode() == false )
    //			return;
    //		std::string packetPackage;
    //		packetPackage += SystemPacketBuilder::CSyncStart::make(
    // myUserInfo_->channel(), myUserInfo_->userId(), "" ); 		packetPackage +=
    // serializeData(); 		packetPackage += serializeJoinerList(); 		packetPackage +=
    // SystemPacketBuilder::CSyncComplete::make( "" );
    //
    //		//sendDataToUsers( packetPackage, toSessionId );
    //	}

    // User Mansagement and Sync

public:
    void toDrawQueue(lib::board::SmartBoardDraw* draw);

private:
    void stopServer(void);

    void _stopFindingServer(void);

    bool startListenBroadCast(void);

    void stopListenBroadCast(void);

    void fireObserver_AddTask(int totalTaskCount, bool playBackWorking) {
        for (std::list<ISharedPaintEvent*>::iterator it = observers_.begin();
             it != observers_.end();
             it++) {
            (*it)->onISharedPaintEvent_AddTask(totalTaskCount, playBackWorking);
        }
    }

    void fireObserver_AddPaintItem(std::shared_ptr<CPaintItem> item) {
        for (std::list<ISharedPaintEvent*>::iterator it = observers_.begin();
             it != observers_.end();
             it++) {
            (*it)->onISharedPaintEvent_AddPaintItem(item);
        }
    }

    void fireObserver_SyncStart(void) {
        enabled_ = false;

        for (std::list<ISharedPaintEvent*>::iterator it = observers_.begin();
             it != observers_.end();
             it++) {
            (*it)->onISharedPaintEvent_SyncStart();
        }
    }

    void fireObserver_SyncComplete(void) {
        enabled_ = true;

        for (std::list<ISharedPaintEvent*>::iterator it = observers_.begin();
             it != observers_.end();
             it++) {
            (*it)->onISharedPaintEvent_SyncComplete();
        }
    }

    void fireObserver_UpdatePaintItem(std::shared_ptr<CPaintItem> item) {
        for (std::list<ISharedPaintEvent*>::iterator it = observers_.begin();
             it != observers_.end();
             it++) {
            (*it)->onISharedPaintEvent_UpdatePaintItem(item);
        }
    }

    void fireObserver_MovePaintItem(std::shared_ptr<CPaintItem> item, double x, double y) {
        for (std::list<ISharedPaintEvent*>::iterator it = observers_.begin();
             it != observers_.end();
             it++) {
            (*it)->onISharedPaintEvent_MovePaintItem(item, x, y);
        }
    }

    void fireObserver_RemovePaintItem(std::shared_ptr<CPaintItem> item) {
        for (std::list<ISharedPaintEvent*>::iterator it = observers_.begin();
             it != observers_.end();
             it++) {
            (*it)->onISharedPaintEvent_RemovePaintItem(item);
        }
    }

    void fireObserver_ResizeMainWindow(int width, int height) {
        lastWindowWidth_ = width;
        lastWindowHeight_ = height;

        //            for (std::list<ISharedPaintEvent *>::iterator it =
        //            observers_.begin(); it != observers_.end(); it++) {
        //                (*it)->onISharedPaintEvent_ResizeMainWindow( width,
        //                height);
        //            }
    }

    void fireObserver_ResizeWindowSplitter(std::vector<int>& sizes) {
        lastWindowSplitterSizes_ = sizes;

        //            for (std::list<ISharedPaintEvent *>::iterator it =
        //            observers_.begin(); it != observers_.end(); it++) {
        //                (*it)->onISharedPaintEvent_ResizeWindowSplitter( sizes);
        //            }
    }

    void fireObserver_SendingPacket(int packetId, size_t wroteBytes, size_t totalBytes) {
        //            for (std::list<ISharedPaintEvent *>::iterator it =
        //            observers_.begin(); it != observers_.end(); it++) {
        //                (*it)->onISharedPaintEvent_SendingPacket( packetId,
        //                wroteBytes, totalBytes);
        //            }
    }

    void fireObserver_ClearScreen(void) {
        for (std::list<ISharedPaintEvent*>::iterator it = observers_.begin();
             it != observers_.end();
             it++) {
            (*it)->onISharedPaintEvent_ClearScreen();
        }

        // clear here for giving a chance to handle current item data.
        clearAllItems();
    }

    void fireObserver_ReceivedPacket(void) {
        for (std::list<ISharedPaintEvent*>::iterator it = observers_.begin();
             it != observers_.end();
             it++) {
            (*it)->onISharedPaintEvent_ReceivedPacket();
        }
    }

    void onDraw(lib::board::SmartBoardDraw*);

    friend class CSharedPaintCommandManager;

    friend class CAddItemTask;

    friend class CRemoveItemTask;

    friend class CUpdateItemTask;

    friend class CMoveItemTask;

    QWidget* painter_;

    CDefferedCaller caller_;
    bool enabled_;
    bool syncStartedFlag_;

    // obsevers
    std::list<ISharedPaintEvent*> observers_;

    // my action command
    std::shared_ptr<CSharedPaintCommandManager> commandMngr_;

    // 面板
    std::shared_ptr<CSharedPainterScene> _scene;
    // 视景
    std::unique_ptr<QGraphicsView> _view;

    std::shared_ptr<PainterEvent> painterEvent_;
    std::shared_ptr<PainterRenderer> painterRenderer_;
    std::shared_ptr<PainterDispatcher> painterDispatcher_;

    // for paint item temporary caching..
    std::shared_ptr<CBackgroundImageItem> backgroundImageItem_;
    int lastWindowWidth_;
    int lastWindowHeight_;
    int lastCanvasWidth_;
    int lastCanvasHeight_;
    std::int16_t lastScrollHPos_;
    std::int16_t lastScrollVPos_;
    std::vector<int> lastWindowSplitterSizes_;
    QColor backgroundColor_;
    int gridLineSize_;

    // user management
    std::recursive_mutex mutexUser_;
    std::shared_ptr<CPaintUser> myUserInfo_;
    //	USER_MAP joinerMap_;
    //	USER_LIST joinerHistory_;

    // network
    enum ConnectionMode { INIT_MODE, PEER_MODE, SERVER_MODE };

    bool findingServerMode_;
    int listenTcpPort_;
    int listenUdpPort_;
    int retryServerReconnectCount_;
    ConnectionMode lastConnectMode_;
    std::string lastConnectAddress_;
    int lastConnectPort_;
    std::string superPeerId_;
    //	SESSION_LIST sessionList_;
    //	std::shared_ptr<CPaintSession> superPeerSession_;
    //	std::shared_ptr<CPaintSession> relayServerSession_;
    //	std::recursive_mutex mutexSession_;
    //	std::shared_ptr<CNetPeerServer> netPeerServer_;
    //	std::shared_ptr< CNetUdpSession > udpSessionForConnection_;
    //	std::shared_ptr< CNetBroadCastSession > broadCastSessionForListener_;
    //	std::shared_ptr< CNetBroadCastSession > broadCastSessionForFinder_;
    //	std::shared_ptr< CNetBroadCastSession > broadCastSessionForSendMessage_;
    //	std::shared_ptr< CNetBroadCastSession > broadCastSessionForRecvMessage_;

    // udp stream
    //	UDP_SESSION_MAP udpSessionMap_;
    //	std::shared_ptr< CNetUdpSession > udpSessionForStream_;
    //	CNetServiceRunner udpStreamRunner_;

    // seding byte management
    //	std::recursive_mutex mutexSendInfo_;
    //	struct send_byte_info_t
    //	{
    //		int wroteBytes;
    //		int totalBytes;
    //		CPaintSession *session;
    //	};
    //	typedef std::map<int, std::vector<struct send_byte_info_t>>
    // send_info_map_t; 	send_info_map_t sendInfoDataMap_;
    int lastPacketId_;

    size_t receives = 0;

    std::unique_ptr<lib::network::NetworkHttp> m_networkManager;

signals:
    //    void draw(lib::board::SmartBoardDraw*);
    void receivedUrlInfo(lib::backend::FileResult);

protected slots:
    void onTimeoutSyncStart(void);

    void onReceivedDropFile(QString);

    void onReceivedScreenCapture(QPixmap pixmap);
};

}  // namespace module::painter
