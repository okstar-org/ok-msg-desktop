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

#ifndef PROFILELOCKER_H
#define PROFILELOCKER_H

#include <QLockFile>
#include <memory>

class ProfileLocker {
private:
    ProfileLocker() = delete;

public:
    static bool isLockable(QString profile);
    static bool lock(QString profile);
    static void unlock();
    static bool hasLock();
    static QString getCurLockName();
    static void assertLock();

private:
    static QString lockPathFromName(const QString& name);
    static void deathByBrokenLock();

private:
    static std::unique_ptr<QLockFile> lockfile;
    static QString curLockName;
};

#endif  // PROFILELOCKER_H
