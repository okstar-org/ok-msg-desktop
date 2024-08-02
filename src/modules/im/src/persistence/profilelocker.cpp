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

#include "profilelocker.h"
#include <QDebug>
#include <QDir>
#include "src/persistence/settings.h"

/**
 * @class ProfileLocker
 * @brief Locks a Tox profile so that multiple instances can not use the same profile.
 * Only one lock can be acquired at the same time, which means
 * that there is little need for manually unlocking.
 * The current lock will expire if you exit or acquire a new one.
 */

using namespace std;

unique_ptr<QLockFile> ProfileLocker::lockfile;
QString ProfileLocker::curLockName;

QString ProfileLocker::lockPathFromName(const QString& name) {
    return Settings::getInstance().getSettingsDirPath() + '/' + name + ".lock";
}

/**
 * @brief Checks if a profile is currently locked by *another* instance.
 * If we own the lock, we consider it lockable.
 * There is no guarantee that the result will still be valid by the
 * time it is returned, this is provided on a best effort basis.
 * @param profile Profile name to check.
 * @return True, if profile locked, false otherwise.
 */
bool ProfileLocker::isLockable(QString profile) {
    // If we already have the lock, it's definitely lockable
    if (lockfile && curLockName == profile) return true;

    QLockFile newLock(lockPathFromName(profile));
    return newLock.tryLock();
}

/**
 * @brief Tries to acquire the lock on a profile, will not block.
 * @param profile Profile to lock.
 * @return Returns true if we already own the lock.
 */
bool ProfileLocker::lock(QString profile) {
    if (lockfile && curLockName == profile) return true;

    QLockFile* newLock = new QLockFile(lockPathFromName(profile));
    newLock->setStaleLockTime(0);
    if (!newLock->tryLock()) {
        delete newLock;
        return false;
    }

    unlock();
    lockfile.reset(newLock);
    curLockName = profile;
    return true;
}

/**
 * @brief Releases the lock on the current profile.
 */
void ProfileLocker::unlock() {
    if (!lockfile) return;

    lockfile->unlock();
    lockfile.reset();
    curLockName.clear();
}

/**
 * @brief Check that we actually own the lock.
 * In case the file was deleted on disk, restore it.
 * If we can't get a lock, exit qTox immediately.
 * If we never had a lock in the first place, exit immediately.
 */
void ProfileLocker::assertLock() {
    if (!lockfile) {
        qCritical() << "assertLock: We don't seem to own any lock!";
        deathByBrokenLock();
    }

    if (!QFile(lockPathFromName(curLockName)).exists()) {
        QString tmp = curLockName;
        unlock();
        if (lock(tmp)) {
            qCritical() << "assertLock: Lock file was lost, but could be restored";
        } else {
            qCritical() << "assertLock: Lock file was lost, and could *NOT* be restored";
            deathByBrokenLock();
        }
    }
}

/**
 * @brief Print an error then exit immediately.
 */
void ProfileLocker::deathByBrokenLock() {
    qCritical() << "Lock is *BROKEN*, exiting immediately";
    abort();
}

/**
 * @brief Chacks, that profile locked.
 * @return Returns true if we're currently holding a lock.
 */
bool ProfileLocker::hasLock() { return lockfile.operator bool(); }

/**
 * @brief Get current locked profile name.
 * @return Return the name of the currently loaded profile, a null string if there is none.
 */
QString ProfileLocker::getCurLockName() {
    if (lockfile)
        return curLockName;
    else
        return QString();
}
