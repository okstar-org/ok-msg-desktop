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

#include <QClipboard>
#include <QString>
#include <QWidget>
#include <memory>

#include "Base.h"
#include "PaintItem.h"
#include "PainterDispatcher.h"
#include "PainterEvent.h"
#include "PainterMdiArea.h"
#include "PainterRenderer.h"
#include "PainterView.h"
#include "SharedPaintManager.h"
#include "SharedPainterScene.h"

#include "lib/backend/BaseService.h"
#include "lib/network/NetworkHttp.h"
#include "src/base/timer.h"
#include "src/painter/OPainterToolBox.h"

namespace module::classroom {

class WhiteboardController;

class PainterView : public QWidget {
public:
    explicit PainterView(QWidget* parent = nullptr);
    ~PainterView() override;

    static PainterView* Get(QWidget* parent = nullptr);

    virtual const CSharedPaintManager* manager() const {
        return painterManager_.get();
    }

    virtual const WhiteboardController* controller() const {
        return _oController.get();
    }

protected:
    void showEvent(QShowEvent* event) override;

    void resizeEvent(QResizeEvent* e) override;

private:
    lib::network::NetworkHttp* networkManager_;

    // 画板管理器
    std::unique_ptr<CSharedPaintManager> painterManager_;

    // 工具箱
    std::unique_ptr<OPainterToolBox> _oToolbox;

    // 控制面板
    std::unique_ptr<WhiteboardController> _oController;

    std::unique_ptr<base::DelayedCallTimer> _delayCaller;

    PainterMdiArea* mdiArea;

signals:

public slots:
    void onReceivedUrlInfo(lib::backend::FileResult);
    void onReceivedDraw(lib::board::SmartBoardDraw*);
    void onOpenFile(std::shared_ptr<CFileItem>);

    void onToolBoxChanged(ToolboxType toolboxType);

    void setTextColor(QColor clr);
    void setTextWeight(int weight);

    void setPenColor(QColor clr);
    void setPenWeight(int weight);

    void onCtrollerChecked(WB_CTRL, bool);

    //  void onPubSubEvent(gloox::PubSub::ItemList &);
};

}  // namespace module::classroom
