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

#ifndef PLUGINITEMFORM_H
#define PLUGINITEMFORM_H

#include <QWidget>
#include "base/timer.h"
#include "lib/backend/OkCloudService.h"
#include "lib/network/NetworkHttp.h"
#include "lib/plugin/PluginInfo.h"

namespace Ui {
class PluginItemForm;
}

class PluginItemForm : public QWidget {
    Q_OBJECT
public:
    explicit PluginItemForm(int row, ok::backend::PluginInfo& pluginInfo,
                            QWidget* parent = nullptr);
    ~PluginItemForm();
    void downLogo();
    void setLogo(const QPixmap& pixmap);
    bool isSetLogo();

protected:
    void showEvent(QShowEvent*);

private:
    Ui::PluginItemForm* ui;
    ok::backend::PluginInfo info;
    int row;
    std::unique_ptr<network::NetworkHttp> http;

signals:
    void loadLogo();
    void logoDownloaded(const QString& fileName, QByteArray& img);

public slots:
    void onLogoDownloaded(const QString& fileName, QByteArray& img);
};

#endif  // PLUGINITEMFORM_H
