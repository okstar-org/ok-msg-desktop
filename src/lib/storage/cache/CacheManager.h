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

//
// Created by gaojie on 25-1-9.
//

#pragma once

#include <QDir>
#include "base/jid.h"

namespace lib::cache {

class CacheManager : public QObject {
    Q_OBJECT
public:
    explicit CacheManager(const QDir& path, QObject* parent = nullptr);
    ~CacheManager() override;

    [[nodiscard]] const QDir& getPath() const {
        return path;
    };
    [[nodiscard]] QByteArray loadAvatarData(const ok::base::Jid& owner) const;
    bool saveAvatarData(const ok::base::Jid& owner, const QByteArray& buf);
    bool deleteAvatarData(const ok::base::Jid& owner);
    [[nodiscard]] QByteArray getAvatarHash(const ok::base::Jid& owner) const;

private:
    QDir path;
};

}  // namespace lib::cache
