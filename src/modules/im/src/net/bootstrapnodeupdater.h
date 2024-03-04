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

#ifndef BOOTSTRAPNODEUPDATER_H
#define BOOTSTRAPNODEUPDATER_H

#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QObject>

#include "src/core/dhtserver.h"

class QNetworkReply;

class BootstrapNodeUpdater : public QObject
{
    Q_OBJECT
public:
    explicit BootstrapNodeUpdater(const QNetworkProxy& proxy, QObject* parent = nullptr);
    void requestBootstrapNodes();
    static QList<DhtServer> loadDefaultBootstrapNodes();

signals:
    void availableBootstrapNodes(QList<DhtServer> nodes);

private slots:
    void onRequestComplete(QNetworkReply* reply);

private:
    static QList<DhtServer> jsonToNodeList(const QJsonDocument& nodeList);
    static void jsonNodeToDhtServer(const QJsonObject& node, QList<DhtServer>& outList);

private:
    QNetworkProxy proxy;
    QNetworkAccessManager nam;
};

#endif // BOOTSTRAPNODEUPDATER_H
