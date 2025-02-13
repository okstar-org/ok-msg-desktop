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

#include "PainterView.h"

#include <memory>

#include <QClipboard>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QMimeData>
#include <QString>
#include <QTextEdit>
#include <QWidget>

#include "lib/board/Controller.h"
#include "lib/board/ControllerSelect.h"
#include "lib/board/ControllerVoice.h"
#include "lib/board/Draw.h"

#include "PaintItem.h"
#include "SharedPaintManager.h"
#include "SharedPainterScene.h"
#include "PainterMdiArea.h"
#include "OPlayerWidget.h"
#include "OVideoViewport.h"
#include "painter/OPainterToolBox.h"

namespace module::classroom {

PainterView::PainterView(QWidget* parent) : QWidget(parent) {
    auto layout = new QVBoxLayout(this);

    // Painter Manager
    painterManager_ = std::make_unique<CSharedPaintManager>(this);

    connect(painterManager_->scene().get(), &CSharedPainterScene::openFile, this,
            &PainterView::onOpenFile);

    //  auto im = lib::IM::Messenger::getInstance()->im();

    // IMSmartBord
    //  _imSmartBoard = im->getSmartBoard();

    //  connect(_imSmartBoard, &smartboard::IMSmartBoard::receivedDraw, this,
    //          &Painter::onReceivedDraw);

    //  connect(_imSmartBoard, SIGNAL(pubsubEvent(gloox::PubSub::ItemList &)),
    //  this, SLOT(onPubSubEvent(gloox::PubSub::ItemList &)));

    mdiArea = new PainterMdiArea(painterManager_->scene(), this);

    layout->addWidget(mdiArea);

    // 工具箱
    _oToolbox = std::make_unique<OPainterToolBox>(this);
    // _oToolbox->move(this->width() - _oToolbox->width() - 20, 20);

    setLayout(layout);

    // 连接工具箱和画板之间的信号
    connect(_oToolbox.get(), &OPainterToolBox::toolChange,  //
            this, &PainterView::onToolBoxChanged);

    // text tool
    connect(_oToolbox.get(), &OPainterToolBox::textColorChange, this, &PainterView::setTextColor);
    connect(_oToolbox.get(), &OPainterToolBox::textWeightChange, this, &PainterView::setTextWeight);

    // pen tool
    connect(_oToolbox.get(), &OPainterToolBox::penColorChange, this, &PainterView::setPenColor);
    connect(_oToolbox.get(), &OPainterToolBox::penWeightChange, this, &PainterView::setPenWeight);

    // 控制按钮
    _oController = std::make_unique<WhiteboardController>(this);
    _oController->move(width() - 320, 20);

    connect(_oController.get(), SIGNAL(checked(WB_CTRL, bool)),
            SLOT(onCtrollerChecked(WB_CTRL, bool)));
}

PainterView::~PainterView() {
    qDebug() << __func__;
}

PainterView* PainterView::Get(QWidget* parent) {
    static PainterView* const painter = new PainterView(parent);
    return painter;
}

void PainterView::showEvent(QShowEvent* event) {
    Q_UNUSED(event);
}

void PainterView::resizeEvent(QResizeEvent* e) {
    painterManager_->setSize(e->size());
    mdiArea->setFixedSize(e->size());
    // _oToolbox->move(this->width() - _oToolbox->width() - 20, 20);
    _oController->move(width() - 320, 20);
}

void PainterView::onReceivedUrlInfo(lib::backend::FileResult _dr) {
    qDebug() << "file:" << _dr.name;
}

void PainterView::onReceivedDraw(lib::board::SmartBoardDraw* draw) {
    painterManager_->toDrawQueue(draw);
}

void PainterView::onOpenFile(std::shared_ptr<CFileItem> file) {
    qDebug() << "file:" << file->name();
    if (file->name().isEmpty()) {
        return;
    }

    lib::backend::FileResult _dr;
    _dr.url = file->url();
    _dr.name = file->name();
    _dr.contentType = file->contentType();

    // 类型
    ok::base::FileContentType contentType_ = ok::base::Files::toContentTypeE(_dr.contentType);

    switch (contentType_) {
        case ok::base::FileContentType::PDF: {
            // auto fv = office::FrameFactory::Create<office::FramePDF>(this);
            // fv->play(_dr.url);
            // fv->move(0, 0);
            // fv->show();

            break;
        }
        case ok::base::FileContentType::VIDEO: {
            auto player = new OPlayerWidget(this);
            player->setSource(QUrl(_dr.url));
            player->play();
            player->resize(800, 600);
            player->show();

#ifndef NO_PAINTER_MDI_AREA
            QPoint pos(0, 0);
            QMdiSubWindow* aSubWin = mdiArea->activeSubWindow();
            if (aSubWin) {
                pos.setX(aSubWin->x() + 20);
                pos.setY(aSubWin->y() + 20);

                if (!mdiArea->rect().contains(pos)) {
                    pos.setX(0);
                    pos.setY(0);
                }
            }

            QMdiSubWindow* subWin = mdiArea->addSubWindow(player);
            subWin->move(pos);
            subWin->show();
#endif

            break;
        }
        default:

            break;
    }
}

void PainterView::onToolBoxChanged(ToolboxType toolboxType) {
    qDebug() << "onToolBoxChanged" << toolboxType;
    painterManager_->setToolType(toolboxType);
}

void PainterView::setTextColor(QColor clr) {
    // qDebug() << clr.name();
    painterManager_->scene()->setTextColor(clr);
}

void PainterView::setTextWeight(int weight) {
    int width = 10 + weight * 4;
    // qDebug() << weight << "width:" << width;
    painterManager_->scene()->setTextWidth(width);
}

void PainterView::setPenColor(QColor clr) {
    // qDebug() << clr.name();
    painterManager_->scene()->setPenColor(clr);
}

void PainterView::setPenWeight(int weight) {
    int width = weight * 2;
    // qDebug() << weight << "width:" << width;
    painterManager_->scene()->setPenWidth(width);
}

void PainterView::onCtrollerChecked(WB_CTRL ctrl, bool checked) {
//    const OVideoViewport* videoViewport = (pClassing->videoViewport());

//    const std::list<UserJID> checkedUsers = videoViewport->isCheckedUsers();

//    if (checkedUsers.empty()) return;

//    std::list<std::string> us;

//    for (auto& e : checkedUsers) {
//        us.push_back(e.node().toStdString());
//    }

//    switch (ctrl) {
//        case WB_CTRL::MOVE:
//            break;
//        case WB_CTRL::MUTE: {
//            std::string id = ok::base::UUID::make().toStdString();
//
//            lib::board::ControllerVoice* voice = new lib::board::ControllerVoice(id);
//
//            if (checked) {
//                voice->setAction(lib::board::Action::OFF);
//            } else
//                voice->setAction(lib::board::Action::ON);
//
//            std::shared_ptr<lib::board::Controller> ctl =
//                    std::make_shared<lib::board::Controller>(id);
//            ctl->addPlugin(voice);
//
//            lib::board::UserList& _t_us = const_cast<lib::board::UserList&>(ctl->userList());
//            std::copy(us.begin(), us.end(), std::back_inserter(_t_us));
//
//            //    _imSmartBoard->sendController(ctl);
//
//            break;
//        }
//        case WB_CTRL::WB:
//            break;
//        case WB_CTRL::RE:
//
//            break;
//        case WB_CTRL::OK:
//
//            break;
//        case WB_CTRL::GIFT:
//
//            break;
//        case WB_CTRL::ALL: {
//            std::string id = ok::base::UUID::make().toStdString();
//
//            lib::board::ControllerSelect* select = new lib::board::ControllerSelect(id);
//
//            if (checked) {
//                select->setAction(lib::board::Action::ON);
//            } else
//                select->setAction(lib::board::Action::OFF);
//
//            std::shared_ptr<lib::board::Controller> ctl =
//                    std::make_shared<lib::board::Controller>(id);
//            ctl->addPlugin(select);
//
//            lib::board::UserList& _t_us = const_cast<lib::board::UserList&>(ctl->userList());
//            std::copy(us.begin(), us.end(), std::back_inserter(_t_us));
//
//            //    _imSmartBoard->sendController(ctl);
//
//            break;
//        }
//    }
}

// void Painter::onPubSubEvent(PubSub::ItemList &items) {
//   DEBUG_LOG(("begin"));
//   for (PubSub::Item *item : items) {
//     DEBUG_LOG(("item id:%1").arg(qstring(item->id())));
//   }
// }

// void Painter::onLogin() {
//   if (networkManager_->userManager()->isTeacher()) {
//     _oController->show();
//   } else {
//     _oController->hide();
//   }
// }
//
// void Painter::onLogout() {}
}  // namespace module::classroom
