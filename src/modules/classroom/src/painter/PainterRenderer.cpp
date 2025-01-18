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

#include "PainterRenderer.h"

namespace module::classroom {

PainterRenderer::PainterRenderer() {}

void PainterRenderer::onISharedPaintEvent_AddTask(int totalTaskCount, bool playBackWorking) {}

void PainterRenderer::onISharedPaintEvent_AddPaintItem(std::shared_ptr<CPaintItem> item) {
    item->setCanvas(_scene);
    item->draw();
}

void PainterRenderer::onISharedPaintEvent_RemovePaintItem(std::shared_ptr<CPaintItem> item) {
    item->remove();
}

void PainterRenderer::onISharedPaintEvent_MovePaintItem(std::shared_ptr<CPaintItem> item, double x,
                                                        double y) {}

void PainterRenderer::onISharedPaintEvent_UpdatePaintItem(std::shared_ptr<CPaintItem> item) {
    item->update();
}

}  // namespace module::classroom
