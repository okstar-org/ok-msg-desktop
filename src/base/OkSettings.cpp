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

//
// Created by gaojie on 23-8-30.
//

#include "OkSettings.h"
#include <QDir>
#include <QMutexLocker>
#include <QObject>
#include <QSettings>
#include <QStandardPaths>
#include <QtWidgets>
#include "base/autorun.h"
#include "system/sys_info.h"

namespace ok::base {

static QStringList locales = {
        "zh_CN",  // 中文简体 zh_CN
        "zh_TW",  // 中文繁体（zh_TW, zh_HK）
        "en",     // 英文   en_US, en_UK
                  //    "es",    // 西班牙语
                  //    "fr",    // 法语
                  //    "ar",    // 阿拉伯语
                  //    "ru",    // 俄语
                  //    "de",    // 德语
                  //    "pt",    // 葡萄牙语
                  //    "it",    // 意大利语
                  //    "ja",    // 日文
                  //    "ko",    // 韩文
};

OkSettings::OkSettings(QObject* parent)  //
        : QObject(parent)
        ,  //
        settingsThread(nullptr)
        ,  //
        currentProfileId(0) {
    settingsThread = new QThread();
    settingsThread->setObjectName(objectName());
    settingsThread->start(QThread::LowPriority);
    moveToThread(settingsThread);

    loadGlobal();
}

void OkSettings::loadGlobal() {
    QString filePath = getGlobalSettingsFile();
    qDebug() << "Loading settings from " + filePath;

    QSettings s(filePath, QSettings::IniFormat);
    s.setIniCodec("UTF-8");

    s.beginGroup("General");
    {
        if (currentProfile.isEmpty()) {
            currentProfile = s.value("currentProfile", "").toString();
            currentProfileId = makeProfileId(currentProfile);
        }

        translation = s.value("translation", true).toString();
        provider = s.value("provider", "").toString();
        showSystemTray = s.value("showSystemTray", true).toBool();
        closeToTray = s.value("closeToTray", false).toBool();
        autostartInTray = s.value("autostartInTray", false).toBool();
        autoSaveEnabled = s.value("autoSaveEnabled", false).toBool();
        minimizeOnClose = s.value("minimizeOnClose", false).toBool();
        minimizeToTray = s.value("minimizeToTray", false).toBool();
    }
    s.endGroup();
}

void OkSettings::saveGlobal() {
    if (QThread::currentThread() != settingsThread)
        return (void)QMetaObject::invokeMethod(&getInstance(), "saveGlobal");

    QMutexLocker locker{&bigLock};

    QString path = ok::base::PlatformInfo::getGlobalSettingsFile();
    qDebug() << "Saving global settings at " + path;

    QSettings s(path, QSettings::IniFormat);
    s.setIniCodec("UTF-8");

    s.clear();
    s.beginGroup("General");
    {
        //
        s.setValue("currentProfile", currentProfile);
        s.setValue("translation", translation);
        s.setValue("provider", provider);
        s.setValue("showSystemTray", showSystemTray);
        s.setValue("closeToTray", closeToTray);
        s.setValue("autostartInTray", autostartInTray);
        s.setValue("autoSaveEnabled", autoSaveEnabled);
        s.setValue("minimizeOnClose", minimizeOnClose);
        s.setValue("minimizeToTray", minimizeToTray);
    }
    s.endGroup();
}

OkSettings& OkSettings::getInstance() {
    static OkSettings* settings = nullptr;
    if (!settings) settings = new OkSettings();
    return *settings;
}

// 国际化下拉框
QStringList OkSettings::getLocales() { return locales; }

QString OkSettings::getTranslation() {
    QMutexLocker locker{&bigLock};
    return translation;
}

void OkSettings::setTranslation(const QString& newValue) {
    QMutexLocker locker{&bigLock};

    if (newValue != translation) {
        translation = newValue;
        emit translationChanged(translation);
    }
}

QDir OkSettings::downloadDir() { return ok::base::PlatformInfo::getAppDownloadDirPath(); }

QDir OkSettings::cacheDir() { return ok::base::PlatformInfo::getAppCacheDirPath(); }

QDir OkSettings::configDir() { return ok::base::PlatformInfo::getAppConfigDirPath(); }

QDir OkSettings::dataDir() { return ok::base::PlatformInfo::getAppDataDirPath(); }

/**
 * @brief Get path to directory, where the application cache are stored.
 * @return Path to application cache, ends with a directory separator.
 */
QDir OkSettings::getAppCacheDirPath() { return PlatformInfo::getAppCacheDirPath(); }

QDir OkSettings::getAppPluginPath() { return PlatformInfo::getAppPluginDirPath(); }

QDir OkSettings::getAppLogPath() { return PlatformInfo::getAppLogDirPath(); }

bool OkSettings::getShowSystemTray() {
    QMutexLocker locker{&bigLock};
    return showSystemTray;
}

void OkSettings::setShowSystemTray(bool newValue) {
    QMutexLocker locker{&bigLock};
    if (newValue != showSystemTray) {
        showSystemTray = newValue;
        emit showSystemTrayChanged(newValue);
    }
}

bool OkSettings::getCloseToTray() {
    QMutexLocker locker{&bigLock};
    return closeToTray;
}

void OkSettings::setCloseToTray(bool newValue) {
    QMutexLocker locker{&bigLock};

    if (newValue != closeToTray) {
        closeToTray = newValue;
        emit closeToTrayChanged(newValue);
    }
}

bool OkSettings::getAutoSaveEnabled() {
    QMutexLocker locker{&bigLock};
    return autoSaveEnabled;
}

void OkSettings::setAutoSaveEnabled(bool newValue) {
    QMutexLocker locker{&bigLock};

    if (newValue != autoSaveEnabled) {
        autoSaveEnabled = newValue;
        emit autoSaveEnabledChanged(autoSaveEnabled);
    }
}

void OkSettings::setAutostartInTray(bool newValue) {
    QMutexLocker locker{&bigLock};
    if (newValue != autostartInTray) {
        autostartInTray = newValue;
        emit autostartInTrayChanged(autostartInTray);
    }
}

bool OkSettings::getMinimizeToTray() {
    QMutexLocker locker{&bigLock};
    return minimizeToTray;
}

void OkSettings::setMinimizeToTray(bool newValue) {
    QMutexLocker locker{&bigLock};
    if (newValue != minimizeToTray) {
        minimizeToTray = newValue;
        emit minimizeToTrayChanged(minimizeToTray);
    }
}

bool OkSettings::getMinimizeOnClose() {
    QMutexLocker locker{&bigLock};
    return minimizeOnClose;
}

void OkSettings::setMinimizeOnClose(bool newValue) {
    QMutexLocker locker{&bigLock};

    if (newValue != minimizeOnClose) {
        minimizeOnClose = newValue;
        emit minimizeOnCloseChanged(minimizeOnClose);
    }
}

bool OkSettings::getAutorun() {
    QMutexLocker locker{&bigLock};
    return Platform::getAutorun();
}

void OkSettings::setAutorun(bool newValue) {
    QMutexLocker locker{&bigLock};
    bool autorun = Platform::getAutorun();
    if (newValue != autorun) {
        Platform::setAutorun(newValue);
        emit autorunChanged(autorun);
    }
}

bool OkSettings::getAutostartInTray() {
    QMutexLocker locker{&bigLock};
    return autostartInTray;
}

QString OkSettings::getCurrentProfile() {
    QMutexLocker locker{&bigLock};
    return currentProfile;
}

uint32_t OkSettings::getCurrentProfileId() {
    QMutexLocker locker{&bigLock};
    return currentProfileId;
}

void OkSettings::setCurrentProfile(const QString& profile) {
    QMutexLocker locker{&bigLock};

    if (profile != currentProfile) {
        currentProfile = profile;
        currentProfileId = makeProfileId(currentProfile);

        emit currentProfileIdChanged(currentProfileId);
    }
}

uint32_t OkSettings::makeProfileId(const QString& profile) {
    QByteArray data = QCryptographicHash::hash(profile.toUtf8(), QCryptographicHash::Md5);
    const uint32_t* dwords = reinterpret_cast<const uint32_t*>(data.constData());
    return dwords[0] ^ dwords[1] ^ dwords[2] ^ dwords[3];
}

QString OkSettings::getGlobalSettingsFile() {
    return ok::base::PlatformInfo::getGlobalSettingsFile();
}

QString OkSettings::getProvider() {
    QMutexLocker locker{&bigLock};
    return provider;
}

void OkSettings::setProvider(QString val) {
    QMutexLocker locker{&bigLock};
    if (val != provider) {
        provider = val;
        emit providerChanged(provider);
    }
}

}  // namespace ok::base
