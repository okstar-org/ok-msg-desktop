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

#include "PluginManagerForm.h"

#include <QDebug>
#include <QListWidgetItem>

#include "PluginInfoForm.h"
#include "PluginItemForm.h"
#include "base/OkSettings.h"
#include "lib/network/NetworkHttp.h"
#include "lib/settings/translator.h"
#include "src/UI/widget/GenericForm.h"
#include "ui_PluginManagerForm.h"

namespace ok {
namespace plugin {

PluginManagerForm::PluginManagerForm(QWidget* parent)
        : UI::GenericForm(QPixmap(":/img/settings/general.png"), parent)
        , ui(new Ui::PluginManagerForm) {
    ui->setupUi(this);

    if (parent) {
        setGeometry(parent->contentsRect());
    }

    //  QString locale = ok::base::OkSettings::getInstance().getTranslation();
    //  settings::Translator::translate(OK_UIWindowConfig_MODULE, "plugin_"+locale);
    //  settings::Translator::registerHandler([this] { retranslateUi(); }, this);
    //  retranslateUi();

    connect(ui->listWidget, &QListWidget::itemClicked, this, &PluginManagerForm::pluginClicked,
            Qt::UniqueConnection);

    delayCaller_ = std::make_unique<::base::DelayedCallTimer>();
    http = std::make_unique<ok::backend::OkCloudService>(this);

    delayCaller_->call(400, [&]() {
        http->GetPluginPage(
                [&](backend::ResPage<ok::backend::PluginInfo>& resList) {
                    int i = 0;
                    for (auto& item : resList.data.list) {
                        qDebug() << "add plugin:" << item.name;
                        add(item, i++);
                    }
                },
                [](int code, const QString& err) { qWarning() << "GetPluginPage" << err; });
    });
}

PluginManagerForm::~PluginManagerForm() { delete ui; }

void PluginManagerForm::pluginClicked(QListWidgetItem* item) {
    Q_UNUSED(item);
    auto wdg = static_cast<QListWidget*>(sender());
    auto mCurrentRow = wdg->currentRow();
    auto info = mPluginInfos.at(mCurrentRow);
    setPluginInfo(info);
}

void PluginManagerForm::add(ok::backend::PluginInfo& info, int i) {
    mPluginInfos.append(info);
    createPlugin(info, i);
}

void PluginManagerForm::createPlugin(ok::backend::PluginInfo& info, int i) {
    auto pitem = new PluginItemForm(0, info, this);
    auto aitem = new QListWidgetItem(ui->listWidget);
    aitem->setSizeHint(QSize(ui->listWidget->width(), 70));

    ui->listWidget->addItem(aitem);
    ui->listWidget->setItemWidget(aitem, pitem);

    retranslateUi();
}

void PluginManagerForm::setPluginInfo(ok::backend::PluginInfo& info) {
    for (auto i = 0; i < ui->stackedWidget->count(); i++) {
        auto entry = static_cast<PluginInfoForm*>(ui->stackedWidget->widget(i));
        if (entry->pluginId() == info.id) {
            ui->stackedWidget->setCurrentWidget(entry);
            return;
        }
    }

    auto selectedInfoForm = new PluginInfoForm(info);
    ui->stackedWidget->addWidget(selectedInfoForm);
    ui->stackedWidget->setCurrentWidget(selectedInfoForm);
}

void PluginManagerForm::retranslateUi() {
    ui->retranslateUi(this);
    for (int i = 0; i < ui->stackedWidget->count(); i++) {
        auto form = static_cast<PluginInfoForm*>(ui->stackedWidget->widget(i));
        form->retranslateUi();
    }
}

}  // namespace plugin
}  // namespace ok
