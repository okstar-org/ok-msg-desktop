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

#ifndef PAINTEREVENT_H
#define PAINTEREVENT_H

#include "Base.h"
#include "PaintItem.h"
#include "SharedPaintManager.h"

#include <QString>

namespace module::classroom {

class PainterEvent : public ICanvasViewEvent {
public:
    PainterEvent();
    virtual ~PainterEvent() override;

    virtual void setManager(CSharedPaintManager* manager) {
        _manager = manager;
    }

protected:
    // ICanvasViewEvent
    void onMoveItem(std::shared_ptr<CPaintItem> item) override;
    void onDrawItem(std::shared_ptr<CPaintItem> item) override;
    void onUpdateItem(std::shared_ptr<CPaintItem> item) override;
    void onRemoveItem(std::shared_ptr<CPaintItem> item) override;
    QString onGetToolTipText(std::shared_ptr<CPaintItem> item) override;
    virtual std::shared_ptr<CPaintItem> onFindItem(ItemId itemId) override;

private:
    CSharedPaintManager* _manager;
};

}  // namespace module::classroom
#endif  // PAINTEREVENT_H
