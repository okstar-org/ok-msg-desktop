#include "webviewpage.h"
#include <QWebEngineView>

ok::platform::WebviewPage::WebviewPage(const QUrl& url, const QString& title,
                                       PlatformPageContainer* container)
        : PlatformPage(container), pageUrl(url), pageTitle(title) {}

ok::platform::WebviewPage::~WebviewPage() {}

QWidget* ok::platform::WebviewPage::getWidget() { return webView; }

void ok::platform::WebviewPage::createContent(QWidget* parent) {
    if (!webView) {
        webView = new QWebEngineView(parent);
        webView->load(pageUrl);
    }
}

void ok::platform::WebviewPage::start() {}

void ok::platform::WebviewPage::doClose() {}
