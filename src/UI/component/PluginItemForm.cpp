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
#include "base/files.h"
#include "lib/network/NetworkHttp.h"
#include "ui_PluginItemForm.h"
#include <QTimer>
#include "base/images.h"

PluginItemForm::PluginItemForm(int row_, ok::backend::PluginInfo &pluginInfo,
                               QWidget *parent)
    : QWidget(parent), ui(new Ui::PluginItemForm), info(pluginInfo), row(row_) {
  ui->setupUi(this);
  ui->name->setText(pluginInfo.name);
  ui->version->setText(pluginInfo.version);
  connect(this, &PluginItemForm::loadLogo, this, &PluginItemForm::timesUp);

  http = std::make_unique<network::NetworkHttp>();
  connect(this, &PluginItemForm::logoDownloaded, this,
          &PluginItemForm::onLogoDownloaded);
}

PluginItemForm::~PluginItemForm() {
  disconnect(this, &PluginItemForm::loadLogo, this, &PluginItemForm::timesUp);
  disconnect(this, &PluginItemForm::logoDownloaded, this,
             &PluginItemForm::onLogoDownloaded);
  delete ui;
}

void PluginItemForm::timesUp() {
  http->get(info.logoUrl, [&](QByteArray img, const QString &fileName) {
    Q_UNUSED(fileName);
    qDebug() << "渲染图片:" << fileName << img.size();
    emit logoDownloaded(fileName, img);
    QTimer::singleShot(100, this, [=, this]() { http.reset(); });
  });
}

void PluginItemForm::setLogo(const QImage &img) {
  ui->logoLabel->setPixmap(QPixmap::fromImage(img));
  ui->logoLabel->setScaledContents(true);
}

void PluginItemForm::onLogoDownloaded(const QString &fileName,
                                      QByteArray &img) {
  Q_UNUSED(fileName)
  QImage image;
  if (base::Images::putToImage(img, image)) {
    setLogo(image);
  }
}