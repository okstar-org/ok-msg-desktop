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

#include "PluginItemForm.h"
#include <QTimer>
#include "base/files.h"
#include "base/images.h"
#include "lib/network/NetworkHttp.h"
#include "ui_PluginItemForm.h"

PluginItemForm::PluginItemForm(int row_, ok::backend::PluginInfo& pluginInfo, QWidget* parent)
        : QWidget(parent), ui(new Ui::PluginItemForm), info(pluginInfo), row(row_) {
    ui->setupUi(this);
    ui->name->setText(pluginInfo.name);
    ui->version->setText(pluginInfo.version);

    http = std::make_unique<network::NetworkHttp>();
    connect(this, &PluginItemForm::logoDownloaded, this, &PluginItemForm::onLogoDownloaded);
}

PluginItemForm::~PluginItemForm() {
    disconnect(this, &PluginItemForm::loadLogo, this, &PluginItemForm::downLogo);
    disconnect(this, &PluginItemForm::logoDownloaded, this, &PluginItemForm::onLogoDownloaded);
    delete ui;
}

void PluginItemForm::downLogo() {
    if (isSetLogo()) return;
    http->get(info.logoUrl, [&](QByteArray img, const QString& fileName) {
        Q_UNUSED(fileName);
        qDebug() << "download image:" << fileName << img.size();
        emit logoDownloaded(fileName, img);
    });
}

void PluginItemForm::setLogo(const QPixmap& pixmap) {
    ui->logoLabel->setPixmap(pixmap);
    ui->logoLabel->setScaledContents(true);
}

bool PluginItemForm::isSetLogo() {
    auto p = ui->logoLabel->pixmap(Qt::ReturnByValueConstant::ReturnByValue);
    return !p.isNull();
}

void PluginItemForm::showEvent(QShowEvent*) { downLogo(); }

void PluginItemForm::onLogoDownloaded(const QString& fileName, QByteArray& img) {
    qDebug() << "logo downloaded" << fileName;
    QPixmap pixmap;
    if (ok::base::Images::putToPixmap(img, pixmap)) {
        setLogo(pixmap);
    }
}
