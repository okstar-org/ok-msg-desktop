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

#include "log/LogManager.h"
#include "settings/OkSettings.h"
#include <QDir>
#include <QObject>


namespace lib::storeage {

class StoreageManager : public QObject {
    Q_OBJECT
public:
    explicit StoreageManager(QObject* parent = nullptr);

    /**
     * 获取存储根目录
     * @brief getDir
     * @return
     */
    const QDir& getDir();

    const log::LogManager& getLogManager() const {
        return logManager;
    }

    const settings::OkSettings& getSettings();

private:
    QDir dir;
    log::LogManager logManager;
};


}
