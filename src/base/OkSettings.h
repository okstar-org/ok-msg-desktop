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
#include "base/compatiblerecursivemutex.h"

namespace ok::base {

class OkSettings : public QObject {
    Q_OBJECT
public:
    explicit OkSettings(QObject* parent = nullptr);
    ~OkSettings() override {}
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

private:
    static uint32_t makeProfileId(const QString& profile);

    QThread* settingsThread;

    CompatibleRecursiveMutex bigLock;
    QString translation;
    QString provider;

    bool showSystemTray;
    bool closeToTray;
    bool autostartInTray;
    bool minimizeToTray;
    bool minimizeOnClose;

    bool autoSaveEnabled;

    QString currentProfile;
    uint32_t currentProfileId;

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

    void currentProfileIdChanged(quint32 id);

public slots:
    void saveGlobal();
};

}  // namespace ok::base
