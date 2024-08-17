#pragma once

#include <QPointer>
#include <QUrl>

namespace ok::platform {

class Platform;
class PlatformPage;
class Widget;

// PlatformPageContainer
// 对外部提供页签打开、删除等操作

class PlatformPageContainer : public QObject {
    Q_OBJECT
public:
    PlatformPageContainer(Platform* platform, Widget* widget);
    // 打开指定url页签
    PlatformPage* openWebPage(const QUrl& webUrl, const QString& pageName = QString());
    // 添加页签
    bool addPage(PlatformPage* page, bool active = false);
    bool removePage(PlatformPage* page);
    bool removePageByUrl(const QUrl& pageUrl);
    void updateTitle();

private:
    QPointer<Widget> container;
    QPointer<Platform> platform;
};
}  // namespace ok::platform