/*
 * Copyright (c) 2022 船山信息 chuanshaninfo.com
 * The project is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PubL v2 for more details.
 */

#ifndef BASE_SYSTEM_SYS_INFO_H_
#define BASE_SYSTEM_SYS_INFO_H_

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QString>
#include <QTextStream>
#include <map>
#include <string>
#include "../basic_types.h"
#include "../utils.h"

namespace ok::base {

static QString GlobalSettingsFileName = APPLICATION_EXE_NAME ".ini";

typedef enum { X86, X64 } Arch;

typedef struct {
    QString manufacturer;
    QString name;
    qint32 cores;
    qint32 processors;
    QString arch;
} CpuInfo;

typedef struct {
    QString name;
    QString version;
    QString prettyName;
    QString kernelName;
    QString kernelVersion;
    QString hostName;
    QString uniqueId;
} OsInfo;

class SysInfo {
public:
    // Returns the host operating system information.
    static bool GetOsInfo(OsInfo& info);

    // Returns the host cpu information.
    static bool GetCpuInfo(CpuInfo& info);
};

class SystemInfo {
public:
    static SystemInfo* instance();
    QString osVersion();
    OsInfo osInfo();
    CpuInfo cpuInfo();
};

class PlatformInfo {
public:
    static bool isPortable() {
        QString localSettingsPath =
                qApp->applicationDirPath() + QDir::separator() + GlobalSettingsFileName;
        return QFile(localSettingsPath).exists();
    }

    static QString getGlobalSettingsFile() {
        QDir dir(ok::base::PlatformInfo::getAppConfigDirPath());
        return dir.filePath(GlobalSettingsFileName);
    }

    static QDir getAppConfigDirPath() {
        if (isPortable())
            return qApp->applicationDirPath() + QDir::separator() + "work_dir" + QDir::separator() +
                   ".config";

        auto p = QDir::cleanPath(
                         QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)) +
                 QDir::separator();
        return ok::base::PathUtils::ensure(p);
    }

    static QDir getAppCacheDirPath() {
        if (isPortable())
            return qApp->applicationDirPath() + QDir::separator() + "work_dir" + QDir::separator() +
                   ".cache";

        auto p = QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)) +
                 QDir::separator();
        return ok::base::PathUtils::ensure(p);
    }

    static QDir getAppDataDirPath() {
        if (isPortable())
            return qApp->applicationDirPath() + QDir::separator() + "work_dir" + QDir::separator() +
                   ".data";

        auto p = QDir::cleanPath(
                         QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)) +
                 QDir::separator();
        return ok::base::PathUtils::ensure(p);
    }

    static QDir getAppDownloadDirPath() {
        if (isPortable())
            return qApp->applicationDirPath() + QDir::separator() + "work_dir" + QDir::separator() +
                   "downloads";

        auto p = QDir::cleanPath(
                         QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)) +
                 QDir::separator() + "downloads";
        return ok::base::PathUtils::ensure(p);
    }

    static QDir getAppLogDirPath() {
        if (isPortable())
            return qApp->applicationDirPath() + QDir::separator() + "work_dir" + QDir::separator() +
                   "logs";

        auto p = QDir::cleanPath(
                         QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)) +
                 QDir::separator() + "logs";
        return ok::base::PathUtils::ensure(p);
    }

    static QDir getAppPluginDirPath() {
        if (isPortable())
            return qApp->applicationDirPath() + QDir::separator() + "work_dir" + QDir::separator() +
                   ".plugins";

        auto p = QDir::cleanPath(
                         QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)) +
                 QDir::separator() + "plugins";
        return ok::base::PathUtils::ensure(p);
    }
};

}  // namespace ok::base

#endif  // BASE_SYSTEM_SYS_INFO_H_
