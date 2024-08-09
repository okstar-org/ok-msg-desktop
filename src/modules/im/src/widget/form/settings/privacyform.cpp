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
#include "ui_privacysettings.h"

#include <QDebug>
#include <QFile>
#include <QMessageBox>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#include <QRandomGenerator>
#endif

#include "lib/settings/translator.h"
#include "src/base/RecursiveSignalBlocker.h"
#include "src/core/core.h"
#include "src/nexus.h"
#include "src/persistence/history.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/widget/form/setpassworddialog.h"
#include "src/widget/form/settingswidget.h"
#include "src/widget/gui.h"
#include "src/widget/widget.h"

PrivacyForm::PrivacyForm()
        : GenericForm(QPixmap(":/img/settings/privacy.png")), bodyUI(new Ui::PrivacySettings) {
    bodyUI->setupUi(this);

    // block all child signals during initialization
    const ok::base::RecursiveSignalBlocker signalBlocker(this);

    eventsInit();
    settings::Translator::registerHandler(std::bind(&PrivacyForm::retranslateUi, this), this);
}

PrivacyForm::~PrivacyForm() {
    settings::Translator::unregister(this);
    delete bodyUI;
}

// void PrivacyForm::on_cbKeepHistory_stateChanged()
//{
//     Settings::getInstance().setEnableLogging(bodyUI->cbKeepHistory->isChecked());
//     if (!bodyUI->cbKeepHistory->isChecked()) {
//         emit clearAllReceipts();
//         QMessageBox::StandardButton dialogDelHistory;
//         dialogDelHistory =
//             QMessageBox::question(nullptr, tr("Confirmation"),
//                                   tr("Do you want to permanently delete all chat history?"),
//                                   QMessageBox::Yes | QMessageBox::No);
//         if (dialogDelHistory == QMessageBox::Yes) {
//             Nexus::getProfile()->getHistory()->eraseHistory();
//         }
//     }
// }
//
// void PrivacyForm::on_cbTypingNotification_stateChanged()
//{
//     Settings::getInstance().setTypingNotification(bodyUI->cbTypingNotification->isChecked());
// }
//
// void PrivacyForm::on_nospamLineEdit_editingFinished()
//{
////    QString newNospam = bodyUI->nospamLineEdit->text();
//
////    bool ok;
////    uint32_t nospam = newNospam.toLongLong(&ok, 16);
////    if (ok)
////        Core::getInstance()->setNospam(nospam);
//}

void PrivacyForm::showEvent(QShowEvent*) {
    const Settings& s = Settings::getInstance();
    //    bodyUI->nospamLineEdit->setText(Core::getInstance()->getSelfId().getNoSpamString());
    bodyUI->cbTypingNotification->setChecked(s.getTypingNotification());
    bodyUI->cbKeepHistory->setChecked(Settings::getInstance().getEnableLogging());
    bodyUI->blackListTextEdit->setText(s.getBlackList().join('\n'));
}
//
// void PrivacyForm::on_randomNosapamButton_clicked()
//{
//    QTime time = QTime::currentTime();
// #if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
//    QRandomGenerator((uint)time.msec());
// #else
//    qsrand((uint)time.msec());
// #endif
//
//    uint32_t newNospam{0};
//    for (int i = 0; i < 4; ++i)
// #if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
//        newNospam = (newNospam << 8) + (QRandomGenerator::global()->generate() % 256); // Generate
//        byte by byte. For some reason.
// #else
//        newNospam = (newNospam << 8) + (qrand() % 256); // Generate byte by byte. For some reason.
// #endif
//
////    Core::getInstance()->setNospam(newNospam);
////    bodyUI->nospamLineEdit->setText(Core::getInstance()->getSelfId().getNoSpamString());
//}

// void PrivacyForm::on_nospamLineEdit_textChanged()
//{
//     QString str = bodyUI->nospamLineEdit->text();
//     int curs = bodyUI->nospamLineEdit->cursorPosition();
//     if (str.length() != 8) {
//         str = QString("00000000").replace(0, str.length(), str);
//         bodyUI->nospamLineEdit->setText(str);
//         bodyUI->nospamLineEdit->setCursorPosition(curs);
//     };
// }

// void PrivacyForm::on_blackListTextEdit_textChanged()
//{
//     const QStringList strlist = bodyUI->blackListTextEdit->toPlainText().split('\n');
//     Settings::getInstance().setBlackList(strlist);
// }

void PrivacyForm::retranslateUi() { bodyUI->retranslateUi(this); }
