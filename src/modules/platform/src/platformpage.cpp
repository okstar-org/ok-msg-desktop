#include "platformpage.h"
#include "platformpagecontainer.h"

using namespace ok::platform;

ok::platform::PlatformPage::PlatformPage(PlatformPageContainer* pageContainer) {
    this->pageContainer = pageContainer;
}

QString PlatformPage::getTitle() { return QString(); }

QIcon PlatformPage::getIcon() { return QIcon(); }

QWidget* PlatformPage::getWidget() { return nullptr; }

void ok::platform::PlatformPage::start() {}

QUrl ok::platform::PlatformPage::getUrl() { return QUrl(); }
