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

#include "StorageManager.h"

#include <QDebug>
#include <memory>
#include "db/rawdatabase.h"
#include "src/base/system/sys_info.h"

namespace lib::storage {

StorageManager::StorageManager(const QString& profile, QObject* parent)
        : QObject(parent), dir(ok::base::PlatformInfo::getAppConfigDirPath()), profile(profile) {
    qDebug() << __func__ << "Initializing for profile:" << profile;
    if (!profile.isEmpty()) {
        // 创建子目录
        QString sub = "ok_"+profile;
        if (!dir.exists(sub)) {
            dir.mkdir(sub);
        }
        dir = QDir(dir.path()+QDir::separator()+sub);
    }
    cacheManager = new cache::CacheManager(dir.path() + QDir::separator() + "cache", this);
    qDebug() << __func__ << "Initialized";
}

StorageManager::~StorageManager() {
    qDebug() << __func__;
}

const QDir& StorageManager::getDir() {
    return dir;
}

std::unique_ptr<settings::OkSettings> StorageManager::getGlobalSettings() const {
    return std::make_unique<settings::OkSettings>();
}

StorageManager* StorageManager::create(const QString& profile_) {
    auto sm = storageMap.value(profile_);
    if (!sm) {
        sm = new StorageManager(profile_);
        storageMap.insert(profile_, sm);
    }
    return sm;
}

db::RawDatabase* StorageManager::createDatabase(const QString& module) {
    QDir dbDir(dir.path() + QDir::separator() + module+".db");
    return new db::RawDatabase(dbDir.path(), QString(), QByteArray{});
}

QSettings* StorageManager::createSetting(const QString &module)
{
    QDir iniDir(dir.path() + QDir::separator() + module+".ini");
    auto ptr = new QSettings(iniDir.path(), QSettings::IniFormat, this);
    ptr->setIniCodec("UTF-8");
    return ptr;
}

}  // namespace lib::storage
