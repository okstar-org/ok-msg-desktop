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

#include "SettingsForm.h"

#include "GeneralForm.h"
#include "UserInterfaceForm.h"
#include "lib/settings/translator.h"
#include "modules/im/src/widget/contentlayout.h"
#include "modules/im/src/widget/form/settings/advancedform.h"
#include "modules/im/src/widget/form/settings/generalform.h"
#include "modules/im/src/widget/form/settings/privacyform.h"

#include <QLabel>
#include <QTabWidget>
#include <QWidget>
#include <QWindow>

#include <memory>

namespace UI {
SettingsWidget::SettingsWidget(QWidget *parent) : QWidget(parent) {
  settingsWidgets = std::unique_ptr<QTabWidget>(new QTabWidget(this));
  settingsWidgets->setTabPosition(QTabWidget::North);

  bodyLayout = std::unique_ptr<QVBoxLayout>(new QVBoxLayout(this));
  bodyLayout->setContentsMargins(0, 0, 0, 0);
  bodyLayout->addWidget(settingsWidgets.get());
  setLayout(bodyLayout.get());

//  std::unique_ptr<GeneralForm> gfrm(new GeneralForm(this));
  //    connect(gfrm.get(), &GeneralForm::updateIcons, parent, &Widget::updateIcons);


  std::unique_ptr<UserInterfaceForm> uifrm(new UserInterfaceForm(this));
  std::unique_ptr<PrivacyForm> pfrm(new PrivacyForm());
  //    connect(pfrm.get(), &PrivacyForm::clearAllReceipts, parent, &Widget::clearAllReceipts);

  std::unique_ptr<AdvancedForm> expfrm(new AdvancedForm());
//  std::unique_ptr<AboutForm> abtfrm(new AboutForm());

#if UPDATE_CHECK_ENABLED
  if (updateCheck != nullptr) {
    connect(updateCheck, &UpdateCheck::updateAvailable, this, &SettingsWidget::onUpdateAvailable);
  } else {
    qWarning() << "SettingsWidget passed null UpdateCheck!";
  }
#endif

  cfgForms = {{
      new GeneralForm(this),   //
//      std::move(uifrm),  //
//      std::move(pfrm),   //
//      std::move(expfrm), //
//      std::move(abtfrm)  //
  }};

  for (auto &cfgForm : cfgForms)
    settingsWidgets->addTab(cfgForm, cfgForm->getFormIcon(), cfgForm->getFormName());
//    settingsWidgets->addTab(gfrm, gfrm->getFormIcon(), gfrm->getFormName());

  connect(settingsWidgets.get(), &QTabWidget::currentChanged, this, &SettingsWidget::onTabChanged);

  settings::Translator::registerHandler([this] { retranslateUi(); }, this);
  retranslateUi();
}

SettingsWidget::~SettingsWidget() { settings::Translator::unregister(this); }

void SettingsWidget::setBodyHeadStyle(QString style) { settingsWidgets->setStyle(QStyleFactory::create(style)); }

void SettingsWidget::showAbout() { onTabChanged(settingsWidgets->count() - 1); }

bool SettingsWidget::isShown() const {
  if (settingsWidgets->isVisible()) {
    settingsWidgets->window()->windowHandle()->alert(0);
    return true;
  }

  return false;
}

void SettingsWidget::show(ContentLayout *contentLayout) {
  //    contentLayout->mainContent->layout()->addWidget(settingsWidgets.get());
  settingsWidgets->show();
  onTabChanged(settingsWidgets->currentIndex());
}

void SettingsWidget::onTabChanged(int index) { settingsWidgets->setCurrentIndex(index); }

void SettingsWidget::onUpdateAvailable(void) {
  //    settingsWidgets->tabBar()->setProperty("update-available", true);
  //    settingsWidgets->tabBar()->style()->unpolish(settingsWidgets->tabBar());
  //    settingsWidgets->tabBar()->style()->polish(settingsWidgets->tabBar());
}

void SettingsWidget::retranslateUi() {
  for (size_t i = 0; i < cfgForms.size(); ++i){
    settingsWidgets->setTabText(i, cfgForms.at(i)->getFormName());
  }
}

} // namespace UI