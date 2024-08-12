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

#include "PluginInfoForm.h"

#include <QDebug>
#include "base/OkSettings.h"
#include "base/files.h"
#include "lib/network/NetworkHttp.h"
#include "lib/plugin/pluginhost.h"
#include "ui_PluginInfoForm.h"

namespace ok {
namespace plugin {

inline QString makePath(const QString& id, const QString& name) {
    return QString("%1/%2_%3").arg(ok::base::OkSettings::downloadDir().path(), id, name);
}

inline QString makePluginPath(const QString& name) {
    return QString("%1/%2").arg(ok::base::OkSettings().getAppPluginPath().path(), name);
}

PluginInfoForm::PluginInfoForm(ok::backend::PluginInfo& info, QWidget* parent)
        : QWidget(parent)
        , ui(new Ui::PluginInfoForm)
        , pluginManager(PluginManager::instance())
        , id(info.id)
        , mPluginInfo(info)
        , downUrl(info.downUrl)
        , mDownloaded(false)
        , isDownloading(false) {
    ui->setupUi(this);
    ui->progressBar->setValue(0);
    ui->name->setText(info.name);
    ui->version->setText(info.version);
    ui->home->setText(info.homeUrl);
    ui->intro->setText(info.content);
    ui->installBtn->setText(tr("Install"));
    ui->installBtn->setCursor(Qt::PointingHandCursor);
    ui->progressBar->hide();

    qDebug() << "Display plugin" << info.shortName;

    if (pluginManager->isAvailable(info.shortName)) {
        qDebug() << "Plugin is available" << info.shortName;
        setInstalled();
    }

    for (const auto& dir : pluginManager->pluginDirs()) {
        qDebug() << "PluginDir:" << dir;
    };

    connect(this, &PluginInfoForm::downloadFinished, [&](const QString& path) {
        QMutexLocker ml{&mMutex};
        http.reset();

        auto installed = pluginManager->installPlugin(path, mPluginInfo.fileName);
        if (installed) setInstalled();
    });
}

PluginInfoForm::~PluginInfoForm() { delete ui; }

void PluginInfoForm::on_installBtn_released() {
    QMutexLocker ml{&mMutex};
    if (mDownloaded) {
        qDebug() << "Is downloaded, will to uninstall the plugin.";
        toUninstall();
    } else {
        qDebug() << "Is not downloaded, will to install the plugin.";
        toInstall();
    }
}

void PluginInfoForm::toUninstall() {
    qDebug() << "To uninstall:" << mPluginInfo.shortName;
    setUninstalling();

    auto uninstallPlugin = pluginManager->uninstallPlugin(mPluginInfo.shortName);
    qDebug() << "To uninstall:" << mPluginInfo.shortName << "=>" << uninstallPlugin;

    if (uninstallPlugin) setUninstalled();
}

void PluginInfoForm::toInstall() {
    qDebug() << "To install:" << mPluginInfo.shortName;

    if (isDownloading) {
        qWarning() << "Is downloading...";
        return;
    }

    qDebug() << "download=>" << downUrl;
    isDownloading = true;
    http = std::make_unique<network::NetworkHttp>();
    http->get(
            downUrl,
            [&](QByteArray buf, const QString& fileName) {
                auto path = makePath(QString::number(id), fileName);
                qDebug() << "download bytes" << buf.size() << "be saved to=>" << path;
                if (!ok::base::Files::writeTo(buf, path)) {
                    qWarning() << "Cannot to write plugin files." << path;
                    return;
                }
                isDownloading = false;
                emit downloadFinished(path);
            },
            [&](qint64 bytesReceived, qint64 bytesTotal) {
                ui->progressBar->setMaximum(bytesTotal);
                ui->progressBar->setValue(bytesReceived);
            },

            [&](int code, const QString& err) {
                qWarning() << "Install code:" << code << err;
                isDownloading = false;
                setUninstalled();
            });

    setInstalling();
}

void PluginInfoForm::setInstalled() {
    mDownloaded = true;
    ui->progressBar->hide();
    ui->installBtn->setText(tr("Uninstall"));
}

void PluginInfoForm::setUninstalled() {
    mDownloaded = false;
    ui->progressBar->hide();
    ui->installBtn->setText(tr("Install"));
}

void PluginInfoForm::setInstalling() {
    mDownloaded = false;
    ui->progressBar->show();
    ui->installBtn->setText(tr("Installing"));
}

void PluginInfoForm::setUninstalling() {
    mDownloaded = false;
    ui->progressBar->show();
    ui->installBtn->setText(tr("Uninstalling"));
}
void PluginInfoForm::retranslateUi() {
    ui->retranslateUi(this);
    ui->installBtn->setText(tr("Install"));
}

}  // namespace plugin
}  // namespace ok
