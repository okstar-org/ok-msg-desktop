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
// Created by gaojie on 24-4-19.
//

#ifndef OKMSG_PROJECT_LOGMANAGER_H
#define OKMSG_PROJECT_LOGMANAGER_H

#include <QDir>
#include <QFile>
#include <QMutex>
#include <memory>

namespace ok {
namespace lib {

class LogManager {
public:
    static const LogManager& Instance();
    static void Destroy();

    [[nodiscard]] const QString& getLogFile() const { return logName; };

    [[nodiscard]] QFile& getFile() const { return *file.get(); }

    ~LogManager();

private:
    LogManager();

    QString logName;
    QDir logFileDir;
    std::unique_ptr<QFile> file;
};

}  // namespace lib
}  // namespace ok

#endif  // OKMSG_PROJECT_LOGMANAGER_H
