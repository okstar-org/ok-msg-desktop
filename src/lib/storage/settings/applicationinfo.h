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

#ifndef APPLICATIONINFO_H
#define APPLICATIONINFO_H

class QLatin1String;
class QString;
class QStringList;

class ApplicationInfo {
public:
    enum HomedirType { ConfigLocation, DataLocation, CacheLocation };

    // Version info
    static QString name();
    static QLatin1String sname();
    static QString version();
    static QString capsNode();
    static QString osName();
    static QString IPCName();

    // URLs
    static QString getAppCastURL();

    // Directories
    static QString homeDir(HomedirType type);
    static QString resourcesDir();
    static QString libDir();
    static QString profilesDir(ApplicationInfo::HomedirType);
    static QString currentProfileDir(HomedirType type);
    static QString makeSubhomePath(const QString&, HomedirType type);
    static QString makeSubprofilePath(const QString&, HomedirType type);
    static QString historyDir();
    static QString vCardDir();
    static QString bobDir();
    static QString documentsDir();
    static QStringList getCertificateStoreDirs();
    static QString getCertificateStoreSaveDir();
    static QStringList dataDirs();
    static QStringList pluginDirs();

    // Namespaces
    static QString optionsNS();
    static QString storageNS();
    static QString fileCacheNS();

    // Common
    static QString desktopFileBaseName();
    static QString desktopFile();
    static bool isPortable();
};

#endif  // APPLICATIONINFO_H
