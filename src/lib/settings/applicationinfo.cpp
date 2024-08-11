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

#include "applicationinfo.h"

#ifdef OK_HAVE_CONFIG
#include "ok_config.h"
#endif
#include "base/OkSettings.h"
#include "base/r.h"
#include "base/system/sys_info.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QLatin1String>
#include <QLocale>
#include <QMessageBox>
#include <QSettings>
#include <QStandardPaths>
#include <QString>
#ifdef Q_OS_MAC
#include <CoreServices/CoreServices.h>
#include <sys/stat.h>  // chmod
#endif
#ifdef Q_OS_UNIX
#include <sys/stat.h>  // chmod
#endif
// #ifdef Q_OS_WIN
// #include <shellapi.h>
// #include <shlobj.h>
// #include <windows.h>
// #endif

#include <QDebug>

#define xstr(a) str(a)
#define str(a) #a

#define PROG_NAME APPLICATION_NAME
#define PROG_SNAME APPLICATION_ALIAS
#define PROG_VERSION GIT_VERSION
#define PROG_DESCRIBE GIT_DESCRIBE
#define PROG_CAPS_NODE CLIENT_CAPS_NODE

#define PROG_IPC_NAME "org.okstar.msg"  // must not contain '\\' character on Windows
#define PROG_OPTIONS_NS "http://okstar.org/options"
#define PROG_STORAGE_NS "http://okstar.org/storage"
#define PROG_FILECACHE_NS "http://okstar.org/filecache"

#ifdef Q_OS_MAC
#define PROG_APPCAST_URL "https://okstar.org/appcast/psi-mac.xml"
#else
#define PROG_APPCAST_URL ""
#endif

QString ApplicationInfo::name() { return PROG_NAME; }

QLatin1String ApplicationInfo::sname() { return QLatin1String(PROG_SNAME); }

QString ApplicationInfo::version() { return APPLICATION_VERSION; }

QString ApplicationInfo::capsNode() { return PROG_CAPS_NODE; }

QString ApplicationInfo::osName() {
    ok::base::OsInfo info;
    ok::base::SysInfo::GetOsInfo(info);
    return info.name;
}

QString ApplicationInfo::IPCName() { return PROG_IPC_NAME; }

QString ApplicationInfo::getAppCastURL() { return PROG_APPCAST_URL; }

QString ApplicationInfo::optionsNS() { return PROG_OPTIONS_NS; }

QString ApplicationInfo::storageNS() { return PROG_STORAGE_NS; }

QString ApplicationInfo::fileCacheNS() { return PROG_FILECACHE_NS; }

QStringList ApplicationInfo::getCertificateStoreDirs() {
#if defined(Q_OS_LINUX) && defined(SHARE_SUFF)
    // Special hack for correct work of AppImage, snap and flatpak builds
    static const QString&& additionalPath =
            QDir().absoluteFilePath(qApp->applicationDirPath() + "/../share/" SHARE_SUFF "/certs");
#endif

    static const QStringList&& dirs = {
#if defined(Q_OS_LINUX) && defined(SHARE_SUFF)
            additionalPath,
#endif
            ApplicationInfo::resourcesDir() + "/certs",
            ApplicationInfo::homeDir(ApplicationInfo::DataLocation) + "/certs"};
    return dirs;
}

QStringList ApplicationInfo::dataDirs() {
#if defined(Q_OS_LINUX) && defined(SHARE_SUFF)
    // Special hack for correct work of AppImage, snap and flatpak builds
    static const QString&& additionalPath =
            QDir().absoluteFilePath(qApp->applicationDirPath() + "/../share/" SHARE_SUFF);
#endif

    static const QStringList&& dirs = {
#if defined(Q_OS_LINUX) && defined(SHARE_SUFF)
            additionalPath,
#endif
            ":", ".", homeDir(DataLocation), resourcesDir()};
    return dirs;
}

QStringList ApplicationInfo::pluginDirs() {
#if defined(Q_OS_LINUX) && defined(SHARE_SUFF)
    // Special hack for correct work of AppImage, snap and flatpak builds
    static const QString&& additionalPath =
            QDir().absoluteFilePath(qApp->applicationDirPath() + "/../lib/" SHARE_SUFF "/plugins");
#endif

    static const QStringList&& dirs = {ok::base::OkSettings().getAppPluginPath().path()};
    return dirs;
}

QString ApplicationInfo::getCertificateStoreSaveDir() {
    QDir certsave(homeDir(DataLocation) + "/certs");
    if (!certsave.exists()) {
        QDir home(homeDir(DataLocation));
        home.mkdir("certs");
    }

    return certsave.path();
}

QString ApplicationInfo::resourcesDir() {
#if defined(Q_OS_WIN)
    return qApp->applicationDirPath();
#elif defined(Q_OS_MAC)
    // FIXME: Clean this up (remko)
    // System routine locates resource files. We "know" that Psi.icns is
    // in the Resources directory.
    QString resourcePath;
    CFBundleRef mainBundle = CFBundleGetMainBundle();
#ifdef PSI_PLUS
    const char* appIconName = "application-plus.icns";
#else
    const char* appIconName = "application.icns";
#endif
    CFStringRef resourceCFStringRef =
            CFStringCreateWithCString(nullptr, appIconName, kCFStringEncodingASCII);
    CFURLRef resourceURLRef =
            CFBundleCopyResourceURL(mainBundle, resourceCFStringRef, nullptr, nullptr);
    if (resourceURLRef) {
        CFStringRef resourcePathStringRef =
                CFURLCopyFileSystemPath(resourceURLRef, kCFURLPOSIXPathStyle);
        const char* resourcePathCString =
                CFStringGetCStringPtr(resourcePathStringRef, kCFStringEncodingASCII);
        if (resourcePathCString) {
            resourcePath = resourcePathCString;
        } else {  // CFStringGetCStringPtr failed; use fallback conversion
            CFIndex bufferLength = CFStringGetLength(resourcePathStringRef) + 1;
            char* resourcePathCString = new char[bufferLength];
            Boolean conversionSuccess =
                    CFStringGetCString(resourcePathStringRef, resourcePathCString, bufferLength,
                                       kCFStringEncodingASCII);
            if (conversionSuccess) {
                resourcePath = resourcePathCString;
            }
            delete[] resourcePathCString;  // I own this
        }
        CFRelease(resourcePathStringRef);  // I own this
    }
    // Remove the tail component of the path
    if (!resourcePath.isNull()) {
        QFileInfo fileInfo(resourcePath);
        resourcePath = fileInfo.absolutePath();
    }
    return resourcePath;
#else
    return OK_DATADIR;
#endif
}

QString ApplicationInfo::libDir() {
#if defined(Q_OS_UNIX) && !defined(Q_OS_HAIKU)
    return OK_LIBDIR;
#else
    return QCoreApplication::applicationDirPath();
#endif
}

/** \brief return psi's private read write data directory
 * unix+mac: $HOME/.psi
 * environment variable "DATADIR" overrides
 */
QString ApplicationInfo::homeDir(ApplicationInfo::HomedirType type) {
    switch (type) {
        case ApplicationInfo::ConfigLocation:
            return ok::base::OkSettings::configDir().path();
        case ApplicationInfo::DataLocation:
            return ok::base::OkSettings::dataDir().path();
        case ApplicationInfo::CacheLocation:
            return ok::base::OkSettings::cacheDir().path();
    }
    return {};
}

QString ApplicationInfo::makeSubhomePath(const QString& path, ApplicationInfo::HomedirType type) {
    if (path.indexOf("..") == -1) {  // ensure its in home dir
        QDir dir(homeDir(type) + "/" + path);
        if (!dir.exists()) {
            dir.mkpath(".");
        }
        return dir.path();
    }
    return QString();
}

QString ApplicationInfo::makeSubprofilePath(const QString& path,
                                            ApplicationInfo::HomedirType type) {
    return QString();
}

QString ApplicationInfo::historyDir() {
    return makeSubprofilePath("history", ApplicationInfo::DataLocation);
}

QString ApplicationInfo::vCardDir() {
    return makeSubprofilePath("vcard", ApplicationInfo::CacheLocation);
}

QString ApplicationInfo::bobDir() { return makeSubhomePath("bob", ApplicationInfo::CacheLocation); }

QString ApplicationInfo::documentsDir() {
    QString docDir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/" +
                   name());
    QDir d(docDir);
    if (!d.exists()) {
        if (!d.mkpath(".")) return QString();
    }
    return docDir;
}

QString ApplicationInfo::profilesDir(ApplicationInfo::HomedirType type) {
    return makeSubhomePath("profiles", type);
}

QString ApplicationInfo::currentProfileDir(ApplicationInfo::HomedirType type) {
    return homeDir(type);
}

QString ApplicationInfo::desktopFileBaseName() {
    return QLatin1String(xstr(APP_BIN_NAME) ".desktop");
}

QString ApplicationInfo::desktopFile() {
    QString dFile;
    auto _desktopFile = QString(xstr(APP_PREFIX) "/share/applications/") + desktopFileBaseName();
    QFile f(_desktopFile);
    if (f.open(QIODevice::ReadOnly)) {
        dFile = QString::fromUtf8(f.readAll());
    }
    return dFile;
}

bool ApplicationInfo::isPortable() {
    static bool portable = QFileInfo(QCoreApplication::applicationFilePath())
                                   .fileName()
                                   .toLower()
                                   .indexOf("portable") != -1;
    return portable;
}
