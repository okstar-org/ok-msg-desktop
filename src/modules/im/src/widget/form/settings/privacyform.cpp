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

#include "privacyform.h"
#include "Bus.h"
#include "ui_privacysettings.h"

#include <QDebug>
#include <QFile>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#include <QRandomGenerator>
#endif

#include "src/base/RecursiveSignalBlocker.h"
#include "src/widget/form/settingswidget.h"
#include "src/application.h"

namespace module::im {

PrivacyForm::PrivacyForm()
        : GenericForm(QPixmap(":/img/settings/privacy.png")), bodyUI(new Ui::PrivacySettings) {
    bodyUI->setupUi(this);

    // block all child signals during initialization
    const ok::base::RecursiveSignalBlocker signalBlocker(this);

    eventsInit();

    auto a = ok::Application::Instance();
    connect(a->bus(), &ok::Bus::languageChanged,this,
            [&](QString locale0) {
                retranslateUi();
            });
}

PrivacyForm::~PrivacyForm() {
    delete bodyUI;
}


void PrivacyForm::showEvent(QShowEvent*) {

}
void PrivacyForm::retranslateUi() {
    //    bodyUI->retranslateUi(this);
}
}  // namespace module::im
