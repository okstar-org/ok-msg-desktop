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

#ifndef PLUGININFOFORM_H
#define PLUGININFOFORM_H

#include <QRecursiveMutex>
#include <QWidget>
#include "lib/backend/OkCloudService.h"
#include "lib/plugin/PluginInfo.h"
#include "lib/plugin/pluginmanager.h"

namespace Ui {
class PluginInfoForm;
}

namespace network {
class NetworkHttp;
}

namespace ok {
namespace plugin {

class PluginInfoForm : public QWidget {
    Q_OBJECT

public:
    explicit PluginInfoForm(ok::backend::PluginInfo& info, QWidget* parent = nullptr);
    ~PluginInfoForm();

    const qint64 pluginId() const { return id; }
    void setInstalling();
    void setInstalled();
    void setUninstalling();
    void setUninstalled();

    void toInstall();
    void toUninstall();
    void retranslateUi();

private:
    Ui::PluginInfoForm* ui;
    PluginManager* pluginManager;
    quint64 id;
    QString downUrl;
    ok::backend::PluginInfo mPluginInfo;
    std::unique_ptr<network::NetworkHttp> http;
    QRecursiveMutex mMutex;

    bool mDownloaded;
    bool isDownloading;

signals:
    void downloadFinished(const QString& path);

private slots:
    void on_installBtn_released();
};
}  // namespace plugin
}  // namespace ok
#endif  // PLUGININFOFORM_H
