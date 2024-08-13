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

#ifndef PLUGINFORM_H
#define PLUGINFORM_H

#include <QMap>
#include <QWidget>
#include "UI/widget/GenericForm.h"
#include "base/timer.h"
#include "lib/backend/OkCloudService.h"
#include "lib/plugin/PluginInfo.h"

namespace Ui {
class PluginManagerForm;
class PluginInfoForm;
}  // namespace Ui

class QListWidgetItem;

namespace ok {
namespace plugin {

class PluginInfoForm;

class PluginManagerForm : public UI::GenericForm {
    Q_OBJECT

public:
    explicit PluginManagerForm(QWidget* parent = nullptr);
    ~PluginManagerForm() override;

    virtual QString getFormName() final override { return tr("Plugin manager"); }

    void add(ok::backend::PluginInfo& info, int i);

    void retranslateUi() override;

private:
    Ui::PluginManagerForm* ui;
    QList<ok::backend::PluginInfo> mPluginInfos;
    std::unique_ptr<::base::DelayedCallTimer> delayCaller_;
    std::unique_ptr<ok::backend::OkCloudService> http;

    void createPlugin(ok::backend::PluginInfo&, int i);
    void setPluginInfo(ok::backend::PluginInfo&);

protected slots:
    void pluginClicked(QListWidgetItem*);
};
}  // namespace plugin
}  // namespace ok
#endif  // PLUGINFORM_H
