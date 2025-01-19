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

#pragma once

#include <QDir>
#include <QObject>
#include <QRect>


#include "base/compatiblerecursivemutex.h"
#include "lib/audio/iaudiosettings.h"
#include "lib/video/ivideosettings.h"

namespace lib::settings {

enum class MainTheme;

class OkSettings : public QObject, public audio::IAudioSettings, public IVideoSettings{
    Q_OBJECT
public:
    explicit OkSettings(QObject* parent = nullptr);
    ~OkSettings() override;
    static OkSettings& getInstance();

    QString getGlobalSettingsFile();

    void loadGlobal();

    QStringList getLocales();
    QString getTranslation();
    void setTranslation(const QString& newValue);

    static QDir downloadDir();
    static QDir cacheDir();
    static QDir configDir();
    static QDir dataDir();

    static QDir getAppCacheDirPath();
    static QDir getAppLogPath();
    static QDir getAppPluginPath();

    bool getShowSystemTray();
    void setShowSystemTray(bool newValue);

    Q_PROPERTY(bool showSystemTray READ getShowSystemTray WRITE setShowSystemTray NOTIFY
                       showSystemTrayChanged FINAL)

    bool getCloseToTray();
    void setCloseToTray(bool newValue);

    Q_PROPERTY(bool autorun READ getAutorun WRITE setAutorun NOTIFY autorunChanged FINAL)
    bool getAutorun();
    void setAutorun(bool newValue);

    bool getAutostartInTray();
    void setAutostartInTray(bool newValue);

    bool getMinimizeToTray();
    void setMinimizeToTray(bool newValue);

    void setAutoSaveEnabled(bool newValue);
    bool getAutoSaveEnabled();

    Q_PROPERTY(bool minimizeOnClose READ getMinimizeOnClose WRITE setMinimizeOnClose NOTIFY
                       minimizeOnCloseChanged FINAL)

    bool getMinimizeOnClose();
    void setMinimizeOnClose(bool newValue);

    QString getCurrentProfile();
    uint32_t getCurrentProfileId();
    void setCurrentProfile(const QString& profile);

    QString getProvider();
    void setProvider(QString val);

    MainTheme getThemeColor();
    void setThemeColor(MainTheme value);

    Q_PROPERTY(QString timestampFormat READ getTimestampFormat WRITE setTimestampFormat NOTIFY
                       timestampFormatChanged FINAL);

    const QString& getTimestampFormat();
    void setTimestampFormat(const QString& format);

    const QString& getDateFormat();
    void setDateFormat(const QString& format);
    Q_PROPERTY(QString dateFormat READ getDateFormat WRITE setDateFormat NOTIFY dateFormatChanged
                       FINAL)

    // State
    QByteArray getWindowGeometry();
    void setWindowGeometry(const QByteArray& value);

    QByteArray getWindowState();
    void setWindowState(const QByteArray& value);

    Q_PROPERTY(QByteArray windowGeometry READ getWindowGeometry WRITE setWindowGeometry NOTIFY
                       windowGeometryChanged FINAL);

    Q_PROPERTY(QByteArray windowState READ getWindowState WRITE setWindowState NOTIFY
                       windowStateChanged FINAL);



            // Audio
    Q_PROPERTY(QString inDev READ getInDev WRITE setInDev NOTIFY inDevChanged FINAL)
    Q_PROPERTY(bool audioInDevEnabled READ getAudioInDevEnabled WRITE setAudioInDevEnabled NOTIFY
                       audioInDevEnabledChanged FINAL)
    Q_PROPERTY(qreal audioInGainDecibel READ getAudioInGainDecibel WRITE setAudioInGainDecibel
                       NOTIFY audioInGainDecibelChanged FINAL)
    Q_PROPERTY(qreal audioThreshold READ getAudioThreshold WRITE setAudioThreshold NOTIFY
                       audioThresholdChanged FINAL)
    Q_PROPERTY(QString outDev READ getOutDev WRITE setOutDev NOTIFY outDevChanged FINAL)
    Q_PROPERTY(bool audioOutDevEnabled READ getAudioOutDevEnabled WRITE setAudioOutDevEnabled NOTIFY
                       audioOutDevEnabledChanged FINAL)
    Q_PROPERTY(int outVolume READ getOutVolume WRITE setOutVolume NOTIFY outVolumeChanged FINAL)
    Q_PROPERTY(int audioBitrate READ getAudioBitrate WRITE setAudioBitrate NOTIFY
                       audioBitrateChanged FINAL)

    QString getInDev() const override;
    void setInDev(const QString& deviceSpecifier) override;

    bool getAudioInDevEnabled() const override;
    void setAudioInDevEnabled(bool enabled) override;

    QString getOutDev() const override;
    void setOutDev(const QString& deviceSpecifier) override;

    bool getAudioOutDevEnabled() const override;
    void setAudioOutDevEnabled(bool enabled) override;

    qreal getAudioInGainDecibel() const override;
    void setAudioInGainDecibel(qreal dB) override;

    qreal getAudioThreshold() const override;
    void setAudioThreshold(qreal percent) override;

    int getOutVolume() const override;
    int getOutVolumeMin() const override { return 0; }
    int getOutVolumeMax() const override { return 100; }
    void setOutVolume(int volume) override;

    int getAudioBitrate() const override;
    void setAudioBitrate(int bitrate) override;

    bool getEnableTestSound() const override;
    void setEnableTestSound(bool newValue) override;

    SIGNAL_IMPL(OkSettings, inDevChanged, const QString& device)
    SIGNAL_IMPL(OkSettings, audioInDevEnabledChanged, bool enabled)

    SIGNAL_IMPL(OkSettings, outDevChanged, const QString& device)
    SIGNAL_IMPL(OkSettings, audioOutDevEnabledChanged, bool enabled)

    SIGNAL_IMPL(OkSettings, audioInGainDecibelChanged, qreal dB)
    SIGNAL_IMPL(OkSettings, audioThresholdChanged, qreal percent)
    SIGNAL_IMPL(OkSettings, outVolumeChanged, int volume)
    SIGNAL_IMPL(OkSettings, audioBitrateChanged, int bitrate)
    SIGNAL_IMPL(OkSettings, enableTestSoundChanged, bool newValue)

            // Video
    Q_PROPERTY(QString videoDev READ getVideoDev WRITE setVideoDev NOTIFY videoDevChanged FINAL)
    Q_PROPERTY(QRect camVideoRes READ getCamVideoRes WRITE setCamVideoRes NOTIFY camVideoResChanged
                       FINAL)
    Q_PROPERTY(QRect screenRegion READ getScreenRegion WRITE setScreenRegion NOTIFY
                       screenRegionChanged FINAL)
    Q_PROPERTY(bool screenGrabbed READ getScreenGrabbed WRITE setScreenGrabbed NOTIFY
                       screenGrabbedChanged FINAL)
    Q_PROPERTY(float camVideoFPS READ getCamVideoFPS WRITE setCamVideoFPS NOTIFY camVideoFPSChanged
                       FINAL)


    QString getVideoDev() const override;
    void setVideoDev(const QString& deviceSpecifier) override;

    QRect getScreenRegion() const override;
    void setScreenRegion(const QRect& value) override;

    bool getScreenGrabbed() const override;
    void setScreenGrabbed(bool value) override;

    QRect getCamVideoRes() const override;
    void setCamVideoRes(QRect newValue) override;

    float getCamVideoFPS() const override;
    void setCamVideoFPS(float newValue) override;

    SIGNAL_IMPL(OkSettings, videoDevChanged, const QString& device)
    SIGNAL_IMPL(OkSettings, screenRegionChanged, const QRect& region)
    SIGNAL_IMPL(OkSettings, screenGrabbedChanged, bool enabled)
    SIGNAL_IMPL(OkSettings, camVideoResChanged, const QRect& region)
    SIGNAL_IMPL(OkSettings, camVideoFPSChanged, unsigned short fps)

private:
    static uint32_t makeProfileId(const QString& profile);

    QThread* settingsThread;

    CompatibleRecursiveMutex bigLock;

    QString translation;
    QString provider;
    MainTheme themeColor;
    QString timestampFormat;
    QString dateFormat;

    bool showSystemTray;
    bool closeToTray;
    bool autostartInTray;
    bool minimizeToTray;
    bool minimizeOnClose;

    bool autoSaveEnabled;

    QString currentProfile;
    quint32 currentProfileId;

    // Window states
    QByteArray windowGeometry;
    QByteArray windowState;
    QByteArray dialogGeometry;

    QString path;

    // Audio 音频
    QString inDev;
    bool audioInDevEnabled;
    qreal audioInGainDecibel;
    qreal audioThreshold;
    QString outDev;
    bool audioOutDevEnabled;
    int outVolume;
    int audioBitrate;
    bool enableTestSound;

    // Video
    QString videoDev;
    QRect camVideoRes;
    QRect screenRegion;
    bool screenGrabbed;
    float camVideoFPS;

signals:
    void translationChanged(const QString& translation);
    void providerChanged(const QString& p);

    void showSystemTrayChanged(bool enabled);

    void closeToTrayChanged(bool enabled);

    void autostartInTrayChanged(bool enabled);

    bool minimizeOnCloseChanged(bool enabled);
    void minimizeToTrayChanged(bool enabled);

    void autorunChanged(bool enabled);
    void autoSaveEnabledChanged(bool enabled);

    void currentProfileChanged(const QString& profile);
    void currentProfileIdChanged(quint32 id);

    void themeColorChanged(MainTheme color);

    void timestampFormatChanged(const QString& format);

    void dateFormatChanged(const QString& format);

    void windowGeometryChanged(const QByteArray& rect);
    void windowStateChanged(const QByteArray& state);
public slots:
    void saveGlobal();
};

}  // namespace lib::settings
