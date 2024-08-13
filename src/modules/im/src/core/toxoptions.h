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

#ifndef TOXOPTIONS_H
#define TOXOPTIONS_H

#include <QByteArray>

#include <memory>

class ICoreSettings;
struct Tox_Options;

class ToxOptions {
public:
    ~ToxOptions();
    ToxOptions(ToxOptions&& from);
    operator Tox_Options*();
    const char* getProxyAddrData() const;
    static std::unique_ptr<ToxOptions> makeToxOptions(const QByteArray& savedata,
                                                      const ICoreSettings* s);
    bool getIPv6Enabled() const;
    void setIPv6Enabled(bool enabled);

private:
    ToxOptions(Tox_Options* options, const QByteArray& proxyAddrData);

private:
    Tox_Options* options = nullptr;
    QByteArray proxyAddrData;
};

#endif  // TOXOPTIONS_H
