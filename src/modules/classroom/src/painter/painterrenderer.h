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

#ifndef PAINTERRENDERER_H
#define PAINTERRENDERER_H

#include "SharedPaintManager.h"
#include "isharedpaintevent.h"

namespace module::classroom {

class PainterRenderer : public ISharedPaintEvent {
public:
    PainterRenderer();

    virtual void setSharedPainterScene(IGluePaintCanvas* scene) {
        _scene = scene;
    }

protected:
    // ISharedPaintEvent
    virtual void onISharedPaintEvent_AddTask(int totalTaskCount, bool playBackWorking) override;
    virtual void onISharedPaintEvent_AddPaintItem(std::shared_ptr<CPaintItem> item) override;
    virtual void onISharedPaintEvent_RemovePaintItem(std::shared_ptr<CPaintItem> item) override;
    virtual void onISharedPaintEvent_MovePaintItem(std::shared_ptr<CPaintItem> item, double x,
                                                   double y) override;

    virtual void onISharedPaintEvent_UpdatePaintItem(std::shared_ptr<CPaintItem> item) override;

private:
    //    std::unique_ptr<CSharedPaintManager> _manager;

    IGluePaintCanvas* _scene;
};

}  // namespace module::classroom
#endif  // PAINTERRENDERER_H
