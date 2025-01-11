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

#pragma once

#include <QDir>
#include <QMap>
#include <QObject>
#include <QSettings>
#include "lib/storage/cache/CacheManager.h"
#include "lib/storage/db/rawdatabase.h"
#include "log/LogManager.h"
#include "settings/OkSettings.h"

namespace lib::storage {

class StorageManager : public QObject {
    Q_OBJECT
public:
    explicit StorageManager(const QString& profile = "", QObject* parent = nullptr);
    ~StorageManager() override;

    const QDir& getDir();

    const log::LogManager& getLogManager() const {
        return logManager;
    }

    cache::CacheManager* getCacheManager() const {
        return cacheManager;
    }

    StorageManager* create(const QString& profile);

    std::unique_ptr<settings::OkSettings> getGlobalSettings() const;

    std::unique_ptr<db::RawDatabase> createDatabase(const QString& module);

    QSettings* createSetting(const QString& module);

private:
    QDir dir;
    log::LogManager logManager;
    cache::CacheManager* cacheManager;
    QString profile;

    // k: profile
    QMap<QString, StorageManager*> storageMap;
};

}  // namespace lib::storage
