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

#include "platformpagecontainer.h"
#include "./pages/webviewpage.h"
#include "Platform.h"
#include "Widget.h"
#include "platformpage.h"

using namespace ok::platform;

PlatformPageContainer::PlatformPageContainer(Platform* platform, Widget* widget)
        : platform(platform), container(widget) {}

PlatformPage* PlatformPageContainer::openWebPage(const QUrl& webUrl,
                                                 const QString& type,
                                                 const QString& uuid,
                                                 const QString& pageName) {
    PlatformPage* page = new WebviewPage(webUrl, type, uuid, pageName, this);
    page->createContent(container.data());
    container->addPage(page, true);
    page->start();
    return page;
}

bool ok::platform::PlatformPageContainer::addPage(PlatformPage* page, bool active) {
    // 不存在时添加
    PlatformPage* existed = container->findPage(page->getUrl());
    if (!existed) {
        container->addPage(page, active);
        return true;
    }
    return false;
}

bool ok::platform::PlatformPageContainer::removePage(PlatformPage* page) {
    if (page) {
        return container->removePage(page);
    }
    return false;
}

bool ok::platform::PlatformPageContainer::removePageByUrl(const QUrl& pageUrl) {
    PlatformPage* page = container->findPage(pageUrl);
    if (page) {
        return container->removePage(page);
    }
    return false;
}

void ok::platform::PlatformPageContainer::updateTitle() {
    // todo: 更新所有page页签的显示
}
