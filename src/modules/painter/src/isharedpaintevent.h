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

#ifndef ISHAREDPAINTEVENT_H
#define ISHAREDPAINTEVENT_H

#include <memory>

#include "PaintItem.h"

namespace module::painter {

class ISharedPaintEvent {
public:
    ISharedPaintEvent() {}
    virtual ~ISharedPaintEvent() {}

    virtual void onISharedPaintEvent_AddTask(int totalTaskCount, bool playBackWorking) = 0;

    virtual void onISharedPaintEvent_AddPaintItem(std::shared_ptr<CPaintItem> item) = 0;

    virtual void onISharedPaintEvent_RemovePaintItem(std::shared_ptr<CPaintItem> item) = 0;

    virtual void onISharedPaintEvent_MovePaintItem(std::shared_ptr<CPaintItem> item, double x,
                                                   double y) = 0;

    virtual void onISharedPaintEvent_UpdatePaintItem(std::shared_ptr<CPaintItem> item) = 0;

    //        virtual void onISharedPaintEvent_ShowErrorMessage( const std::string &error) {}

    virtual void onISharedPaintEvent_Connected() {}

    virtual void onISharedPaintEvent_ConnectFailed() {}

    //        virtual void onISharedPaintEvent_SendingPacket( int packetId, size_t wroteBytes,
    //        size_t totalBytes) {}

    virtual void onISharedPaintEvent_ReceivedPacket() {}

    virtual void onISharedPaintEvent_Disconnected() {}

    virtual void onISharedPaintEvent_SyncStart() {}

    virtual void onISharedPaintEvent_SyncComplete() {}

    //        virtual void onISharedPaintEvent_ResizeMainWindow( int width, int height) {}

    //        virtual void onISharedPaintEvent_ResizeCanvas( int width, int height) {}

    //        virtual void onISharedPaintEvent_ChangeCanvasScrollPos( int posH, int posV) {}

    //        virtual void onISharedPaintEvent_ResizeWindowSplitter( std::vector<int> &sizes) {}

    virtual void onISharedPaintEvent_ClearScreen() {}

    virtual void onISharedPaintEvent_ClearBackground() {}
};

}  // namespace module::painter
#endif  // ISHAREDPAINTEVENT_H
