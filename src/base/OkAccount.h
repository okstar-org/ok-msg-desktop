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
// Created by gaojie on 23-8-19.
//

#pragma once

#include <QObject>
#include "jid.h"
namespace ok::base {

class OkAccount : public QObject {
    Q_OBJECT
public:
    explicit OkAccount(const QString& username);

    [[nodiscard]] inline const QString& name() const { return username; }

    [[nodiscard]] inline const QString& getUsername() const { return username; }

    [[maybe_unused]] inline const base::Jid& jid() const { return m_jid; }

    void setJid(const base::Jid& jid);

    [[maybe_unused]] base::Jid realJid(base::Jid jid);
    [[maybe_unused]] QString id() const { return username; }

private:
    QString username;

    base::Jid m_jid;
};


}  // namespace ok::base

