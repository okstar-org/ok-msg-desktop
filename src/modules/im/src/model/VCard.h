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
// Created by gaojie on 24-10-25.
//

#pragma once

#include <QString>
#include <QStringList>

struct VCard {
    struct Adr {
        QString street;
        QString locality;
        QString region;
        QString country;

        QString location() const { return region + locality; }
    };

    struct Tel {
        int type;  // 0:home,1:work
        bool mobile;
        QString number;
    };

    struct Email {
        int type;  // 0:home,1:work
        QString number;
    };

    struct Photo {
        QString type;
        std::string bin;
        QString url;
    };

    QString fullName;
    QString nickname;
    QString title;
    QList<Adr> adrs;
    QList<Email> emails;
    QList<Tel> tels;
    Photo photo;
};
