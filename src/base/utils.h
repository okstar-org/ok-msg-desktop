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

#include <QDateTime>
#include <QDir>
#include <QObject>

#include "basic_types.h"

namespace ok::base {

class KeyUtils {
public:
    static QString GetISOTime() {
        return QDateTime::currentDateTime().toString(Qt::DateFormat::ISODate);
    }

    static QString GetTimestamp() {
        QString ts = QString("%1").arg(QDateTime::currentDateTime().toMSecsSinceEpoch());
        return ts;
    }
};

class PathUtils {
public:
    [[nodiscard]] inline static QDir ensure(const QString& path) {
        QDir dir(path);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        return dir;
    }
};

}  // namespace ok::base
