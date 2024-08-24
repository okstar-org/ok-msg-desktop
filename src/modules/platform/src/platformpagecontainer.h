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
    PlatformPage* openWebPage(const QUrl& webUrl, const QString& uuid,
                              const QString& pageName = QString());
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