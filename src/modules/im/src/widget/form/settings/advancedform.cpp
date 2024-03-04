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

#include "advancedform.h"
#include "ui_advancedsettings.h"

#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>

#include "src/model/status.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/widget/gui.h"
#include "src/widget/tool/recursivesignalblocker.h"
#include "lib/settings/translator.h"

/**
 * @class AdvancedForm
 *
 * This form contains all connection settings.
 * Is also contains "Reset settings" button and "Make portable" checkbox.
 */

AdvancedForm::AdvancedForm()
    : GenericForm(QPixmap(":/img/settings/general.png"))
    , bodyUI(new Ui::AdvancedSettings)
{
    bodyUI->setupUi(this);

    // block all child signals during initialization
    const RecursiveSignalBlocker signalBlocker(this);

    Settings& s = Settings::getInstance();
    bodyUI->cbEnableIPv6->setChecked(s.getEnableIPv6());
    bodyUI->proxyAddr->setText(s.getProxyAddr());
    quint16 port = s.getProxyPort();
    if (port > 0) {
        bodyUI->proxyPort->setValue(port);
    }

    int index = static_cast<int>(s.getProxyType());
    bodyUI->proxyType->setCurrentIndex(index);
    on_proxyType_currentIndexChanged(index);
    const bool udpEnabled = !s.getForceTCP() && (s.getProxyType() == Settings::ProxyType::ptNone);
    bodyUI->cbEnableUDP->setChecked(udpEnabled);
    bodyUI->cbEnableLanDiscovery->setChecked(s.getEnableLanDiscovery() && udpEnabled);
    bodyUI->cbEnableLanDiscovery->setEnabled(udpEnabled);


    eventsInit();
    settings::Translator::registerHandler(std::bind(&AdvancedForm::retranslateUi, this), this);
}

AdvancedForm::~AdvancedForm()
{
    settings::Translator::unregister(this);
    delete bodyUI;
}


void AdvancedForm::on_btnExportLog_clicked()
{
    QString savefile =
        QFileDialog::getSaveFileName(Q_NULLPTR, tr("Save File"), QString{}, tr("Logs (*.log)"));

    if (savefile.isNull() || savefile.isEmpty()) {
        qDebug() << "Debug log save file was not properly chosen";
        return;
    }

    QString logFileDir = Settings::getInstance().getAppCacheDirPath();
    QString logfile = logFileDir + "qtox.log";

    QFile file(logfile);
    if (file.exists()) {
        qDebug() << "Found debug log for copying";
    } else {
        qDebug() << "No debug file found";
        return;
    }

    if (QFile::copy(logfile, savefile))
        qDebug() << "Successfully copied to: " << savefile;
    else
        qDebug() << "File was not copied";
}

void AdvancedForm::on_btnCopyDebug_clicked()
{
    QString logFileDir = Settings::getInstance().getAppCacheDirPath();
    QString logfile = logFileDir + "qtox.log";

    QFile file(logfile);
    if (!file.exists()) {
        qDebug() << "No debug file found";
        return;
    }

    QClipboard* clipboard = QApplication::clipboard();
    if (clipboard) {
        QString debugtext;
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            debugtext = in.readAll();
            file.close();
        } else {
            qDebug() << "Unable to open file for copying to clipboard";
            return;
        }

        clipboard->setText(debugtext, QClipboard::Clipboard);
        qDebug() << "Debug log copied to clipboard";
    } else {
        qDebug() << "Unable to access clipboard";
    }
}

void AdvancedForm::on_resetButton_clicked()
{
    const QString titile = tr("Reset settings");
    bool result = GUI::askQuestion(titile, tr("All settings will be reset to default. Are you sure?"),
                                   tr("Yes"), tr("No"));

    if (!result)
        return;

    Settings::getInstance().resetToDefault();
    GUI::showInfo(titile, "Changes will take effect after restart");
}

void AdvancedForm::on_cbEnableIPv6_stateChanged()
{
    Settings::getInstance().setEnableIPv6(bodyUI->cbEnableIPv6->isChecked());
}

void AdvancedForm::on_cbEnableUDP_stateChanged()
{
    const bool enableUdp = bodyUI->cbEnableUDP->isChecked();
    Settings::getInstance().setForceTCP(!enableUdp);
    const bool enableLanDiscovery = Settings::getInstance().getEnableLanDiscovery();
    bodyUI->cbEnableLanDiscovery->setEnabled(enableUdp);
    bodyUI->cbEnableLanDiscovery->setChecked(enableUdp && enableLanDiscovery);
}

void AdvancedForm::on_cbEnableLanDiscovery_stateChanged()
{
    Settings::getInstance().setEnableLanDiscovery(bodyUI->cbEnableLanDiscovery->isChecked());
}

void AdvancedForm::on_proxyAddr_editingFinished()
{
    Settings::getInstance().setProxyAddr(bodyUI->proxyAddr->text());
}

void AdvancedForm::on_proxyPort_valueChanged(int port)
{
    if (port <= 0) {
        port = 0;
    }

    Settings::getInstance().setProxyPort(port);
}

void AdvancedForm::on_proxyType_currentIndexChanged(int index)
{
    Settings::ProxyType proxytype = static_cast<Settings::ProxyType>(index);
    const bool proxyEnabled = proxytype != Settings::ProxyType::ptNone;

    bodyUI->proxyAddr->setEnabled(proxyEnabled);
    bodyUI->proxyPort->setEnabled(proxyEnabled);
    // enabling UDP and proxy can be a privacy issue
    bodyUI->cbEnableUDP->setEnabled(!proxyEnabled);
    bodyUI->cbEnableUDP->setChecked(!proxyEnabled);

    Settings::getInstance().setProxyType(proxytype);
}

/**
 * @brief Retranslate all elements in the form.
 */
void AdvancedForm::retranslateUi()
{
    int proxyType = bodyUI->proxyType->currentIndex();
    bodyUI->retranslateUi(this);
    bodyUI->proxyType->setCurrentIndex(proxyType);
}
