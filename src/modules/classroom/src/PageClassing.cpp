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

#include "PageClassing.h"
#include "Classroom.h"
#include "OVideoViewport.h"
#include "src/painter/OPainterViewport.h"

#include <QResizeEvent>

namespace module::classroom {

PageClassing::PageClassing(QWidget* parent) : ok::base::Page(parent) {
    auto layout = new QGridLayout(this);

    auto video = new OVideoViewport(this);
    layout->addWidget(video);

    auto viewport = new OPainterViewport(this);
    layout->addWidget(viewport);

    setLayout(layout);
}

PageClassing::~PageClassing() {}

const OVideoViewport* PageClassing::videoViewport() const {
    //    return ui->video_viewport;
    return nullptr;
}

void PageClassing::toggleChat(bool checked) {
    //    ui->painter_viewport->toggleChat(checked);
}

void PageClassing::resizeEvent(QResizeEvent* e) {}

}  // namespace module::classroom
