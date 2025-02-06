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
// Created by gaojie on 23-8-30.
//

#include "OkSettings.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QDir>
#include <QMutexLocker>
#include <QObject>
#include <QSettings>
#include <QStandardPaths>
#include <QThread>

#include "base/autorun.h"
#include "base/system/sys_info.h"
#include "style.h"

namespace lib::settings {

static QStringList locales = {
        "zh_CN",  // 中文简体 zh_CN
        "zh_TW",  // 中文繁体（zh_TW, zh_HK）
        "en",     // 英文   en_US, en_UK
        "ja",     // 日文
        "ko",     // 韩文
        "de",     // 德语
        "fr",     // 法语
        "es",     // 西班牙语
        "ar",     // 阿拉伯语
        "pt",     // 葡萄牙语
        "it",     // 意大利语
        "ru",     // 俄语
};

OkSettings::OkSettings(QObject* parent)
        : QObject(parent)
        , settingsThread(nullptr)
        , currentProfileId(0)
        , themeColor(MainTheme::Light){
    settingsThread = new QThread();
    settingsThread->setObjectName(objectName());
    settingsThread->start(QThread::LowPriority);
    moveToThread(settingsThread);

    qRegisterMetaType<MainTheme>("MainTheme");
    connect(this, &OkSettings::themeColorChanged, this, &OkSettings::saveGlobal);
    connect(this, &OkSettings::windowGeometryChanged, this, &OkSettings::saveGlobal);
    connect(this, &OkSettings::timestampFormatChanged, this, &OkSettings::saveGlobal);
    connect(this, &OkSettings::dateFormatChanged, this, &OkSettings::saveGlobal);
    connect(this, &OkSettings::outVolumeChanged, this, &OkSettings::saveGlobal);
    connect(this, &OkSettings::camVideoResChanged, this, &OkSettings::saveGlobal);
    connect(this, &OkSettings::camVideoFPSChanged, this, &OkSettings::saveGlobal);

    path = getGlobalSettingsFile();
    qDebug() << "Settings file at:" << path;
    loadGlobal();
}

OkSettings::~OkSettings() {
    qDebug() << __func__;
}

void OkSettings::loadGlobal() {
    QMutexLocker locker{&bigLock};

    QSettings s(path, QSettings::IniFormat, this);
    qDebug() << "Loaded global settings at:" << path;
    s.setIniCodec("UTF-8");
    s.beginGroup("General");
    {
        if (currentProfile.isEmpty()) {
            currentProfile = s.value("currentProfile", "").toString();
            currentProfileId = makeProfileId(currentProfile);
        }

        translation = s.value("translation", true).toString();
        provider = s.value("provider", "").toString();
        themeColor = static_cast<MainTheme>(s.value("themeColor", 0).toInt());
        showSystemTray = s.value("showSystemTray", true).toBool();
        closeToTray = s.value("closeToTray", false).toBool();
        autostartInTray = s.value("autostartInTray", false).toBool();
        autoSaveEnabled = s.value("autoSaveEnabled", false).toBool();
        minimizeOnClose = s.value("minimizeOnClose", false).toBool();
        minimizeToTray = s.value("minimizeToTray", false).toBool();
        timestampFormat = s.value("timestampFormat", "hh:mm:ss").toString();
        dateFormat = s.value("dateFormat", "yyyy-MM-dd").toString();
    }
    s.endGroup();

    s.beginGroup("State");
    {
        windowGeometry = s.value("windowGeometry", QByteArray()).toByteArray();
        windowState = s.value("windowState", QByteArray()).toByteArray();
        dialogGeometry = s.value("dialogGeometry", QByteArray()).toByteArray();
    }
    s.endGroup();


    // 音频
    s.beginGroup("Audio");
    {
        inDev = s.value("inDev", "").toString();
        audioInDevEnabled = s.value("audioInDevEnabled", true).toBool();
        audioInGainDecibel = s.value("audioInGainDecibel", 0).toReal();
        outDev = s.value("outDev", "").toString();
        audioOutDevEnabled = s.value("audioOutDevEnabled", true).toBool();
        audioThreshold = s.value("audioThreshold", 0).toReal();
        outVolume = s.value("outVolume", 100).toInt();
        enableTestSound = s.value("enableTestSound", true).toBool();
        audioBitrate = s.value("audioBitrate", 64).toInt();
    }
    s.endGroup();

    s.beginGroup("Video");
    {
        videoDev = s.value("videoDev", "").toString();
        camVideoRes = s.value("camVideoRes", QRect()).toRect();
        screenRegion = s.value("screenRegion", QRect()).toRect();
        screenGrabbed = s.value("screenGrabbed", false).toBool();
        camVideoFPS = static_cast<quint16>(s.value("camVideoFPS", 0).toUInt());
    }
    s.endGroup();


}

void OkSettings::saveGlobal() {

    qDebug() << __func__;

    QMutexLocker locker{&bigLock};
    qDebug() << "Loaded global settings at:" << path;

    QSettings s(path, QSettings::IniFormat, this);
    s.setIniCodec("UTF-8");

    s.clear();
    s.beginGroup("General");
    {
        s.setValue("currentProfile", currentProfile);
        s.setValue("translation", translation);
        s.setValue("provider", provider);
        s.setValue("showSystemTray", showSystemTray);
        s.setValue("closeToTray", closeToTray);
        s.setValue("autostartInTray", autostartInTray);
        s.setValue("autoSaveEnabled", autoSaveEnabled);
        s.setValue("minimizeOnClose", minimizeOnClose);
        s.setValue("minimizeToTray", minimizeToTray);
        s.setValue("themeColor", static_cast<int>(themeColor));
    }
    s.endGroup();

    s.beginGroup("State");
    {
        s.setValue("windowGeometry", windowGeometry);
        s.setValue("windowState", windowState);
        s.setValue("dialogGeometry", dialogGeometry);
    }
    s.endGroup();

    s.beginGroup("Video");
    {
        s.setValue("videoDev", videoDev);
        s.setValue("camVideoRes", camVideoRes);
        s.setValue("camVideoFPS", camVideoFPS);
        s.setValue("screenRegion", screenRegion);
        s.setValue("screenGrabbed", screenGrabbed);
    }
    s.endGroup();

    s.beginGroup("Audio");
    {
        s.setValue("inDev", inDev);
        s.setValue("audioInDevEnabled", audioInDevEnabled);
        s.setValue("audioInGainDecibel", audioInGainDecibel);
        s.setValue("outDev", outDev);
        s.setValue("outVolume", outVolume);
        s.setValue("audioOutDevEnabled", audioOutDevEnabled);
        s.setValue("audioThreshold", audioThreshold);
        s.setValue("enableTestSound", enableTestSound);
        s.setValue("audioBitrate", audioBitrate);
    }
    s.endGroup();

    qDebug() << "Saved global settings at:" << path;
}

OkSettings& OkSettings::getInstance() {
    static OkSettings* settings = nullptr;
    if (!settings) settings = new OkSettings();
    return *settings;
}

// 国际化下拉框
QStringList OkSettings::getLocales() {
    return locales;
}

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

QDir OkSettings::downloadDir() {
    return ok::base::PlatformInfo::getAppDownloadDirPath();
}

QDir OkSettings::cacheDir() {
    return ok::base::PlatformInfo::getAppCacheDirPath();
}

QDir OkSettings::configDir() {
    return ok::base::PlatformInfo::getAppConfigDirPath();
}

QDir OkSettings::dataDir() {
    return ok::base::PlatformInfo::getAppDataDirPath();
}

QDir OkSettings::getAppCacheDirPath() {
    return ok::base::PlatformInfo::getAppCacheDirPath();
}

QDir OkSettings::getAppPluginPath() {
    return ok::base::PlatformInfo::getAppPluginDirPath();
}

QDir OkSettings::getAppLogPath() {
    return ok::base::PlatformInfo::getAppLogDirPath();
}

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

MainTheme OkSettings::getThemeColor() {
    QMutexLocker locker{&bigLock};
    return themeColor;
}

void OkSettings::setThemeColor(MainTheme value) {
    QMutexLocker locker{&bigLock};
    if (value != themeColor) {
        themeColor = value;
        emit themeColorChanged(themeColor);
    }
}

const QString& OkSettings::getTimestampFormat() {
    QMutexLocker locker{&bigLock};
    return timestampFormat;
}

void OkSettings::setTimestampFormat(const QString& format) {
    QMutexLocker locker{&bigLock};

    if (format != timestampFormat) {
        timestampFormat = format;
        emit timestampFormatChanged(timestampFormat);
    }
}

const QString& OkSettings::getDateFormat() {
    QMutexLocker locker{&bigLock};
    return dateFormat;
}

void OkSettings::setDateFormat(const QString& format) {
    QMutexLocker locker{&bigLock};

    if (format != dateFormat) {
        dateFormat = format;
        emit dateFormatChanged(dateFormat);
    }
}

QByteArray OkSettings::getWindowGeometry() {
    QMutexLocker locker{&bigLock};
    return windowGeometry;
}

void OkSettings::setWindowGeometry(const QByteArray& value) {
    QMutexLocker locker{&bigLock};

    if (value != windowGeometry) {
        windowGeometry = value;
        emit windowGeometryChanged(windowGeometry);
    }
}

QByteArray OkSettings::getWindowState() {
    QMutexLocker locker{&bigLock};
    return windowState;
}

void OkSettings::setWindowState(const QByteArray& value) {
    QMutexLocker locker{&bigLock};

    if (value != windowState) {
        windowState = value;
        emit windowStateChanged(windowState);
    }
}



QString OkSettings::getInDev() const {
    // QMutexLocker locker{&bigLock};
    return inDev;
}

void OkSettings::setInDev(const QString& deviceSpecifier) {
    QMutexLocker locker{&bigLock};

    if (deviceSpecifier != inDev) {
        inDev = deviceSpecifier;
        emit inDevChanged(inDev);
    }
}

bool OkSettings::getAudioInDevEnabled() const {
    // QMutexLocker locker(&bigLock);
    return audioInDevEnabled;
}

void OkSettings::setAudioInDevEnabled(bool enabled) {
    QMutexLocker locker(&bigLock);

    if (enabled != audioInDevEnabled) {
        audioInDevEnabled = enabled;
        emit audioInDevEnabledChanged(enabled);
    }
}

qreal OkSettings::getAudioInGainDecibel() const {
    // QMutexLocker locker{&bigLock};
    return audioInGainDecibel;
}

void OkSettings::setAudioInGainDecibel(qreal dB) {
    QMutexLocker locker{&bigLock};

    if (dB < audioInGainDecibel || dB > audioInGainDecibel) {
        audioInGainDecibel = dB;
        emit audioInGainDecibelChanged(audioInGainDecibel);
    }
}

qreal OkSettings::getAudioThreshold() const {
    // QMutexLocker locker{&bigLock};
    return audioThreshold;
}

void OkSettings::setAudioThreshold(qreal percent) {
    QMutexLocker locker{&bigLock};

    if (percent < audioThreshold || percent > audioThreshold) {
        audioThreshold = percent;
        emit audioThresholdChanged(audioThreshold);
    }
}

QString OkSettings::getVideoDev() const {
    // QMutexLocker locker{&bigLock};
    return videoDev;
}

void OkSettings::setVideoDev(const QString& deviceSpecifier) {
    QMutexLocker locker{&bigLock};

    if (deviceSpecifier != videoDev) {
        videoDev = deviceSpecifier;
        emit videoDevChanged(videoDev);
    }
}

bool OkSettings::getEnableTestSound() const {
    // QMutexLocker locker{&bigLock};
    return enableTestSound;
}

void OkSettings::setEnableTestSound(bool newValue) {
    QMutexLocker locker{&bigLock};

    if (newValue != enableTestSound) {
        enableTestSound = newValue;
        emit enableTestSoundChanged(enableTestSound);
    }
}

QString OkSettings::getOutDev() const {
    // QMutexLocker locker{&bigLock};
    return outDev;
}

void OkSettings::setOutDev(const QString& deviceSpecifier) {
    QMutexLocker locker{&bigLock};

    if (deviceSpecifier != outDev) {
        outDev = deviceSpecifier;
        emit outDevChanged(outDev);
    }
}

bool OkSettings::getAudioOutDevEnabled() const {
    // QMutexLocker locker(&bigLock);
    return audioOutDevEnabled;
}

void OkSettings::setAudioOutDevEnabled(bool enabled) {
    QMutexLocker locker(&bigLock);

    if (enabled != audioOutDevEnabled) {
        audioOutDevEnabled = enabled;
        emit audioOutDevEnabledChanged(audioOutDevEnabled);
    }
}

int OkSettings::getOutVolume() const {
    // QMutexLocker locker{&bigLock};
    return outVolume;
}

void OkSettings::setOutVolume(int volume) {
    QMutexLocker locker{&bigLock};

    if (volume != outVolume) {
        outVolume = volume;
        emit outVolumeChanged(outVolume);
    }
}

int OkSettings::getAudioBitrate() const {
    // const QMutexLocker locker{&bigLock};
    return audioBitrate;
}

void OkSettings::setAudioBitrate(int bitrate) {
    const QMutexLocker locker{&bigLock};

    if (bitrate != audioBitrate) {
        audioBitrate = bitrate;
        emit audioBitrateChanged(audioBitrate);
    }
}

QRect OkSettings::getScreenRegion() const {
    // QMutexLocker locker(&bigLock);
    return screenRegion;
}

void OkSettings::setScreenRegion(const QRect& value) {
    QMutexLocker locker{&bigLock};

    if (value != screenRegion) {
        screenRegion = value;
        emit screenRegionChanged(screenRegion);
    }
}

bool OkSettings::getScreenGrabbed() const {
    // QMutexLocker locker(&bigLock);
    return screenGrabbed;
}

void OkSettings::setScreenGrabbed(bool value) {
    QMutexLocker locker{&bigLock};

    if (value != screenGrabbed) {
        screenGrabbed = value;
        emit screenGrabbedChanged(screenGrabbed);
    }
}

QRect OkSettings::getCamVideoRes() const {
    // QMutexLocker locker{&bigLock};
    return camVideoRes;
}

void OkSettings::setCamVideoRes(QRect newValue) {
    QMutexLocker locker{&bigLock};

    if (newValue != camVideoRes) {
        camVideoRes = newValue;
        emit camVideoResChanged(camVideoRes);
    }
}

float OkSettings::getCamVideoFPS() const {
    // QMutexLocker locker{&bigLock};
    return camVideoFPS;
}

void OkSettings::setCamVideoFPS(float newValue) {
    QMutexLocker locker{&bigLock};

    if (newValue != camVideoFPS) {
        camVideoFPS = newValue;
        emit camVideoFPSChanged(camVideoFPS);
    }
}

}  // namespace lib::settings
