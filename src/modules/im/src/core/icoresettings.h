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

#ifndef I_CORE_SETTINGS_H
#define I_CORE_SETTINGS_H

#include "src/base/interface.h"

#include <QList>
#include <QString>

class QNetworkProxy;

class ICoreSettings {
public:
    enum class ProxyType {
        // If changed, don't forget to update Settings::fixInvalidProxyType
        ptNone = 0,
        ptSOCKS5 = 1,
        ptHTTP = 2
    };
    virtual ~ICoreSettings() = default;

    virtual bool getEnableIPv6() const = 0;
    virtual void setEnableIPv6(bool enable) = 0;

    virtual bool getForceTCP() const = 0;
    virtual void setForceTCP(bool enable) = 0;

    virtual bool getEnableLanDiscovery() const = 0;
    virtual void setEnableLanDiscovery(bool enable) = 0;

    virtual QString getProxyAddr() const = 0;
    virtual void setProxyAddr(const QString& address) = 0;

    virtual ProxyType getProxyType() const = 0;
    virtual void setProxyType(ProxyType type) = 0;

    virtual quint16 getProxyPort() const = 0;
    virtual void setProxyPort(quint16 port) = 0;

    virtual QNetworkProxy getProxy() const = 0;

    DECLARE_SIGNAL(enableIPv6Changed, bool enabled);
    DECLARE_SIGNAL(forceTCPChanged, bool enabled);
    DECLARE_SIGNAL(enableLanDiscoveryChanged, bool enabled);
    DECLARE_SIGNAL(proxyTypeChanged, ICoreSettings::ProxyType type);
    DECLARE_SIGNAL(proxyAddressChanged, const QString& address);
    DECLARE_SIGNAL(proxyPortChanged, quint16 port);
};

#endif  // I_CORE_SETTINGS_H
