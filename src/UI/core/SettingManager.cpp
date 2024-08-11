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
#include "SettingManager.h"

#include "base/r.h"

#include <QDebug>

#define LOGIN_ACCOUNT_KEY "LOGIN_account"
#define LOGIN_PASSWORD_KEY "LOGIN_password"

namespace ok {

SettingManager::SettingManager(QObject* parent)
        : QObject(parent)
        , settings{std::make_unique<QSettings>(ORGANIZATION_NAME, ORGANIZATION_DOMAIN)} {
    qDebug() << __func__;
}

SettingManager::~SettingManager() { qDebug() << __func__; }

void SettingManager::saveAccount(QString& account, QString& password) {
    settings->setValue(LOGIN_ACCOUNT_KEY, QVariant(account));
    settings->setValue(LOGIN_PASSWORD_KEY, QVariant(password));
}

void SettingManager::getAccount(ok::base::Fn<void(QString account, QString password)> callback) {
    QString a = settings->value(LOGIN_ACCOUNT_KEY, QVariant("")).toString();
    QString p = settings->value(LOGIN_PASSWORD_KEY, QVariant("")).toString();

    callback(a, p);
}

void SettingManager::clearAccount() {
    settings->setValue(LOGIN_ACCOUNT_KEY, QVariant(""));
    settings->setValue(LOGIN_PASSWORD_KEY, QVariant(""));
}
}  // namespace ok
