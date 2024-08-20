#include "platformpagecontainer.h"
#include "./pages/webviewpage.h"
#include "Platform.h"
#include "Widget.h"
#include "platformpage.h"

using namespace ok::platform;

PlatformPageContainer::PlatformPageContainer(Platform* platform, Widget* widget)
        : platform(platform), container(widget) {}

PlatformPage* PlatformPageContainer::openWebPage(const QUrl& webUrl, const QString& pageName) {
    PlatformPage* page = new WebviewPage(webUrl, pageName, this);
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
