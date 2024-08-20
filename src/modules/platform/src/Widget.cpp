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

//
// Created by gaojie on 24-7-31.
//

#include "Widget.h"
#include "ui_Widget.h"

#include <QTabBar>
#include "AppCenterWidget.h"
#include "Bus.h"
#include "platformpage.h"
#include "application.h"
#include "base/OkSettings.h"
#include "lib/settings/translator.h"

#include <QAbstractButton>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>

class TabCloseButton : public QAbstractButton {
public:
    explicit TabCloseButton(QWidget* parent = nullptr);
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override { return sizeHint(); }
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
};

namespace ok::platform {

Widget::Widget(QWidget* parent) : UI::OMenuWidget(parent), ui(new Ui::WorkPlatform) {
    OK_RESOURCE_INIT(Platform);
    ui->setupUi(this);
    ui->tabWidget->setObjectName("mainTab");

    QString locale = ok::base::OkSettings::getInstance().getTranslation();
    settings::Translator::translate(OK_Platform_MODULE, locale);
    settings::Translator::registerHandler([this] { retranslateUi(); }, this);
    retranslateUi();
    connect(ok::Application::Instance()->bus(), &ok::Bus::languageChanged,
            [](QString locale0) { settings::Translator::translate(OK_Platform_MODULE, locale0); });

    //    thread = (std::make_unique<QThread>());
    //    thread->setObjectName("WorkPlatform");
    //    connect(thread.get(), &QThread::started, this, &Widget::doStart);
    //    moveToThread(thread.get());
    //
}

Widget::~Widget() {
    settings::Translator::unregister(this);

    const int count = ui->tabWidget->count();
    QList<PlatformPage*> pages;
    for (int i = 0; i < count; i++) {
        pages << ui->tabWidget->tabBar()->tabData(i).value<PlatformPage*>();
    }
    for (PlatformPage* page : pages) {
        delete page;
    }
    delete ui;
}

void Widget::start() {
    const int count = ui->tabWidget->count();
    for (int i = 0; i < count; i++) {
        PlatformPage* page = ui->tabWidget->tabBar()->tabData(i).value<PlatformPage*>();
        if (page) {
            page->start();
        }
    }
}

PlatformPage* Widget::findPage(const QUrl& url) {
    const int count = ui->tabWidget->count();
    for (int i = 0; i < count; i++) {
        PlatformPage* page = ui->tabWidget->tabBar()->tabData(i).value<PlatformPage*>();
        if (page && page->getUrl() == url) {
            return page;
        }
    }
    return nullptr;
}

void Widget::addPage(PlatformPage* page, bool active) {
    if (!page) {
        return;
    }

    QString title = page->getTitle();
    QWidget* w = page->getWidget();
    if (w) {
        int index = ui->tabWidget->indexOf(w);
        if (index < 0)
        {
            index = ui->tabWidget->addTab(w, title);
            ui->tabWidget->tabBar()->setTabData(index, QVariant::fromValue<PlatformPage*>(page));
            if (page->pageClosable()) {
                TabCloseButton* button = new TabCloseButton();
                ui->tabWidget->tabBar()->setTabButton(index, QTabBar::RightSide, button);
                connect(button, &TabCloseButton::clicked, this, &Widget::requestCloseTab);
            }
            
        }
        if (active && index >= 0) {
            ui->tabWidget->setCurrentIndex(index);
        }
    }
}

bool Widget::removePage(PlatformPage* page) {
    const int count = ui->tabWidget->count();
    for (int i = 0; i < count; i++) {
        PlatformPage* temp = ui->tabWidget->tabBar()->tabData(i).value<PlatformPage*>();
        if (temp && temp == page) {
            doClose(i, temp);
            return true;
        }
    }
    return false;
}

void Widget::activePage(PlatformPage* page) {
    if (page && page->getWidget()) {
        ui->tabWidget->setCurrentWidget(page->getWidget());
    }
}

void Widget::doStart() {}

void Widget::retranslateUi() {
    ui->retranslateUi(this);

    const int count = ui->tabWidget->count();
    for (int i = 0; i < count; i++) {
        PlatformPage* page = ui->tabWidget->tabBar()->tabData(i).value<PlatformPage*>();
        if (page) {
            ui->tabWidget->setTabText(i, page->getTitle());
        }
    }
}

void Widget::requestCloseTab() {
    QObject* sender = this->sender();
    const int count = ui->tabWidget->count();
    for (int i = 0; i < count; i++) {
        QWidget* button = ui->tabWidget->tabBar()->tabButton(i, QTabBar::RightSide);
        if (button == sender) {
            PlatformPage* page = ui->tabWidget->tabBar()->tabData(i).value<PlatformPage*>();
            doClose(i, page);
            break;
        }
    }
}

void Widget::doClose(int index, PlatformPage* page) {
    QWidget* widget = ui->tabWidget->widget(index);
    Q_ASSERT(widget);
    Q_ASSERT(page);
    page->doClose();
    ui->tabWidget->removeTab(index);
    widget->hide();
    widget->setParent(nullptr);
    widget->deleteLater();
    delete page;
}

}  // namespace ok::platform

TabCloseButton::TabCloseButton(QWidget* parent) : QAbstractButton(parent) {
    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::ArrowCursor);
    setToolTip(ok::platform::Widget::tr("Close Tab"));
    resize(sizeHint());
}

QSize TabCloseButton::sizeHint() const {
    ensurePolished();
    int width = style()->pixelMetric(QStyle::PM_TabCloseIndicatorWidth, nullptr, this);
    int height = style()->pixelMetric(QStyle::PM_TabCloseIndicatorHeight, nullptr, this);
    return QSize(width, height);
}

void TabCloseButton::enterEvent(QEvent* event) {
    if (isEnabled()) {
        update();
    }
    QAbstractButton::enterEvent(event);
}

void TabCloseButton::leaveEvent(QEvent* event) {
    if (isEnabled()) {
        update();
    }
    QAbstractButton::leaveEvent(event);
}

void TabCloseButton::paintEvent(QPaintEvent*) {
    QPainter p(this);
    QStyleOption opt;
    opt.init(this);
    opt.state |= QStyle::State_AutoRaise;
    if (isEnabled() && underMouse() && !isChecked() && !isDown()) {
        opt.state |= QStyle::State_Raised;
    }
    if (isChecked()) {
        opt.state |= QStyle::State_On;
    }
    if (isDown()) {
        opt.state |= QStyle::State_Sunken;
    }
    if (const QTabBar* tb = qobject_cast<const QTabBar*>(parent())) {
        int index = tb->currentIndex();
        QTabBar::ButtonPosition position = (QTabBar::ButtonPosition)style()->styleHint(
                QStyle::SH_TabBar_CloseButtonPosition, nullptr, tb);
        if (tb->tabButton(index, position) == this) {
            opt.state |= QStyle::State_Selected;
        }
    }
    style()->drawPrimitive(QStyle::PE_IndicatorTabClose, &opt, &p, this);
}