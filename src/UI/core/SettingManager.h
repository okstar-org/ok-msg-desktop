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

#include <QObject>
#include <QSettings>
#include <QString>
#include <memory>

#include <base/basic_types.h>

namespace ok {

class SettingManager : public QObject {
    Q_OBJECT

public:
    SettingManager(QObject* parent = nullptr);
    ~SettingManager();

    void saveAccount(QString& account, QString& password);
    void getAccount(ok::base::Fn<void(QString account, QString password)> callback);
    void clearAccount();

private:
    std::unique_ptr<QSettings> settings;
};
}  // namespace ok
