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

#include "settingswidget.h"


#include <QLabel>
#include <QStyle>
#include <QTabBar>
#include <QTabWidget>
#include <QWindow>

#include <memory>

#include "src/application.h"
#include "src/nexus.h"
#include "src/widget/contentlayout.h"
#include "src/widget/form/settings/StorageSettingsForm.h"
#include "src/widget/form/settings/avform.h"
#include "src/widget/form/settings/generalform.h"
#include "src/widget/widget.h"
#include "lib/storage/settings/translator.h"

namespace module::im {
SettingsWidget::SettingsWidget(QWidget* parent) : QWidget(parent, Qt::Window) {
    //    IAudioSettings* audioSettings = Nexus::getProfile()->getSettings();
    //    IVideoSettings* videoSettings = Nexus::getProfile()->getSettings();

    settingsWidgets = std::unique_ptr<QTabWidget>(new QTabWidget(this));
    settingsWidgets->setTabPosition(QTabWidget::North);

    bodyLayout = std::unique_ptr<QVBoxLayout>(new QVBoxLayout(this));
    bodyLayout->setContentsMargins(0, 0, 0, 0);
    bodyLayout->addWidget(settingsWidgets.get());
    setLayout(bodyLayout.get());

    std::unique_ptr<GeneralForm> gfrm(new GeneralForm(this));
    //    connect(gfrm.get(), &GeneralForm::updateIcons, parent, &Widget::updateIcons);

    std::unique_ptr<StorageSettingsForm> uifrm(new StorageSettingsForm(this));
    //    std::unique_ptr<PrivacyForm> pfrm(new PrivacyForm());
    //    connect(pfrm.get(), &PrivacyForm::clearAllReceipts, parent, &Widget::clearAllReceipts);

    AVForm* rawAvfrm = new AVForm();
    std::unique_ptr<AVForm> avfrm(rawAvfrm);

#if UPDATE_CHECK_ENABLED
    if (updateCheck != nullptr) {
        connect(updateCheck, &UpdateCheck::updateAvailable, this,
                &SettingsWidget::onUpdateAvailable);
    } else {
        qWarning() << "SettingsWidget passed null UpdateCheck!";
    }
#endif

    cfgForms.push_back(std::move(gfrm));
    cfgForms.push_back(std::move(uifrm));
    cfgForms.push_back(std::move(avfrm));

    for (auto& cfgForm : cfgForms)
        settingsWidgets->addTab(cfgForm.get(), cfgForm->getFormIcon(), cfgForm->getFormName());

    connect(settingsWidgets.get(), &QTabWidget::currentChanged, this,
            &SettingsWidget::onTabChanged);

    auto a = ok::Application::Instance();
    connect(a->bus(), &ok::Bus::languageChanged,this,
            [&](const QString& locale0) {
                retranslateUi();
            });
}

SettingsWidget::~SettingsWidget() {
    
}

void SettingsWidget::setBodyHeadStyle(QString style) {
    settingsWidgets->setStyle(QStyleFactory::create(style));
}

void SettingsWidget::showAbout() {
    onTabChanged(settingsWidgets->count() - 1);
}

bool SettingsWidget::isShown() const {
    if (settingsWidgets->isVisible()) {
        settingsWidgets->window()->windowHandle()->alert(0);
        return true;
    }

    return false;
}

void SettingsWidget::show(ContentLayout* contentLayout) {
    //    contentLayout->mainContent->layout()->addWidget(settingsWidgets.get());
    settingsWidgets->show();
    onTabChanged(settingsWidgets->currentIndex());
}

void SettingsWidget::onTabChanged(int index) {
    settingsWidgets->setCurrentIndex(index);
}

void SettingsWidget::onUpdateAvailable() {
    settingsWidgets->tabBar()->setProperty("update-available", true);
    settingsWidgets->tabBar()->style()->unpolish(settingsWidgets->tabBar());
    settingsWidgets->tabBar()->style()->polish(settingsWidgets->tabBar());
}

void SettingsWidget::retranslateUi() {
    auto& settings = lib::settings::OkSettings::getInstance();
    auto locale = settings.getTranslation();
    settings::Translator::translate(OK_IM_MODULE, locale);

    for (size_t i = 0; i < cfgForms.size(); ++i){
        auto n = cfgForms[i]->getFormName();
        settingsWidgets->setTabText(i, n);
    }
}
}  // namespace module::im
