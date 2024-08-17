#pragma once

#include "../platformpage.h"

class QWebEngineView;

namespace ok::platform {

class PlatformPageContainer;

class WebviewPage : public PlatformPage {
public:
    WebviewPage(const QUrl& url, const QString& title, PlatformPageContainer* container);
    ~WebviewPage();
    QString getTitle() override { return pageTitle; }
    QUrl getUrl() override { return pageUrl; }
    QWidget* getWidget() override;

    void createContent(QWidget* parent) override;
    void start() override;
    void doClose() override;

private:
    QUrl pageUrl;
    QString pageTitle;
    QWebEngineView* webView = nullptr;
};
}  // namespace ok::platform