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

#ifndef PAINTERDISPATCHER_H
#define PAINTERDISPATCHER_H

#include "isharedpaintevent.h"

#include "lib/board/smartboarddraw.h"
#include "lib/network/NetworkHttp.h"

namespace module::painter {

class PainterDispatcher : public ISharedPaintEvent {
public:
    PainterDispatcher();
    virtual ~PainterDispatcher() {}

    //    virtual void setManager(CSharedPaintManager* manager){
    //        _manager = manager;
    //    }

    //    virtual void draw(lib::board::SmartBoardDraw *draw);

protected:
    // ISharedPaintEvent
    virtual void onISharedPaintEvent_AddTask(int totalTaskCount, bool playBackWorking) override;
    virtual void onISharedPaintEvent_AddPaintItem(std::shared_ptr<CPaintItem> item) override;
    virtual void onISharedPaintEvent_RemovePaintItem(std::shared_ptr<CPaintItem> item) override;
    virtual void onISharedPaintEvent_MovePaintItem(std::shared_ptr<CPaintItem> item, double x,
                                                   double y) override;
    virtual void onISharedPaintEvent_UpdatePaintItem(std::shared_ptr<CPaintItem> item) override;

private:
    //    lib::board::SmartBoard* _imSmartBoard;
    QObject* _imSmartBoard;
    //    CSharedPaintManager* _manager;
};

}  // namespace module::painter
#endif  // PAINTERDISPATCHER_H
