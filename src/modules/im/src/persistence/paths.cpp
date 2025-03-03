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

#include "paths.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QString>
#include <QStringBuilder>
#include <QStringList>

namespace {
const QLatin1String globalSettingsFile{"qtox.ini"};
const QLatin1String profileFolder{"profiles"};
const QLatin1String themeFolder{"themes"};
const QLatin1String avatarsFolder{"avatars"};
const QLatin1String transfersFolder{"transfers"};
const QLatin1String screenshotsFolder{"screenshots"};

// NOTE(sudden6): currently unused, but reflects the TCS at 2018-11
#ifdef Q_OS_WIN
const QLatin1String TCSToxFileFolder{"%APPDATA%/tox/"};
#elif defined(Q_OS_OSX)
const QLatin1String TCSToxFileFolder{"~/Library/Application Support/Tox"};
#else
const QLatin1String TCSToxFileFolder{"~/.config/tox/"};
#endif
}  // namespace

/**
 * @brief Paths::makePaths Factory method for the Paths object
 * @param mode
 * @return Pointer to Paths object on success, nullptr else
 */
Paths* Paths::makePaths(Portable mode) {
    bool portable = false;
    const QString basePortable = qApp->applicationDirPath();
    const QString baseNonPortable =
            QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString portableSettingsPath = basePortable % QDir::separator() % globalSettingsFile;

    switch (mode) {
        case Portable::Portable:
            qDebug() << "Forcing portable";
            portable = true;
            break;
        case Portable::NonPortable:
            qDebug() << "Forcing non-portable";
            portable = false;
            break;
        case Portable::Auto:
            // auto detect
            if (QFile{portableSettingsPath}.exists()) {
                qDebug() << "Automatic portable";
                portable = true;
            } else {
                qDebug() << "Automatic non-portable";
                portable = false;
            }
            break;
    }

    QString basePath = portable ? basePortable : baseNonPortable;

    if (basePath.isEmpty()) {
        qCritical() << "Couldn't find writeable path";
        return nullptr;
    }

    return new Paths(basePath, portable);
}

Paths::Paths(const QString& basePath, bool portable) : basePath{basePath}, portable{portable} {}

bool Paths::isPortable() const { return portable; }

/**
 * @brief Returns the path to the global settings file "qtox.ini"
 * @return The path to the folder.
 */
QString Paths::getGlobalSettingsPath() const {
    QString path;

    if (portable) {
        path = basePath;
    } else {
        path = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
        if (path.isEmpty()) {
            qDebug() << "Can't find writable location for settings file";
            return {};
        }
    }

    // we assume a writeable path for portable mode

    return path % QDir::separator() % globalSettingsFile;
}

/**
 * @brief Get the folder where profile specific information is stored, e.g. <profile>.ini
 * @return The path to the folder.
 */
QString Paths::getProfilesDir() const {
    return basePath % QDir::separator() % profileFolder % QDir::separator();
}

/**
 * @brief Get the folder where the <profile>.tox file is stored
 * @note Expect a change here, since TCS will probably be updated.
 * @return The path to the folder on success, empty string else.
 */
QString Paths::getToxSaveDir() const {
    if (isPortable()) {
        return basePath % QDir::separator() % profileFolder % QDir::separator();
    }

    // GenericDataLocation would be a better solution, but we keep this code for backward
    // compatibility

// workaround for https://bugreports.qt-project.org/browse/QTBUG-38845
#ifdef Q_OS_WIN
    // TODO(sudden6): this doesn't really follow the Tox Client Standard and probably
    // breaks when %APPDATA% is changed
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +
                           QDir::separator() + "AppData" + QDir::separator() + "Roaming" +
                           QDir::separator() + "tox") +
           QDir::separator();
#elif defined(Q_OS_OSX)
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +
                           QDir::separator() + "Library" + QDir::separator() +
                           "Application Support" + QDir::separator() + "Tox") +
           QDir::separator();
#else
    // TODO(sudden6): This does not respect the XDG_* environment variables and also
    // stores user data in a config location
    return QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
                           QDir::separator() + "tox") +
           QDir::separator();
#endif
}

/**
 * @brief Get the folder where avatar files are stored
 * @note Expect a change here, since TCS will probably be updated.
 * @return The path to the folder on success, empty string else.
 */
QString Paths::getAvatarsDir() const {
    // follow the layout in
    // https://tox.gitbooks.io/tox-client-standard/content/data_storage/export_format.html
    QString path = getToxSaveDir();
    if (path.isEmpty()) {
        qDebug() << "Can't find location for avatars directory";
        return {};
    }

    return path % avatarsFolder % QDir::separator();
}

/**
 * @brief Get the folder where screenshots are stored
 * @return The path to the folder.
 */
QString Paths::getScreenshotsDir() const {
    return basePath % QDir::separator() % screenshotsFolder % QDir::separator();
}

/**
 * @brief Get the folder where file transfer data is stored
 * @return The path to the folder.
 */
QString Paths::getTransfersDir() const {
    return basePath % QDir::separator() % transfersFolder % QDir::separator();
}

/**
 * @brief Get a prioritized list with directories that contain themes.
 * @return A list of directories sorted from most important to least important.
 * @note Users of this function should use the theme from the folder that appears first in the list.
 */
QStringList Paths::getThemeDirs() const {
    QStringList themeFolders{};

    if (!isPortable()) {
        themeFolders += QStandardPaths::locate(QStandardPaths::AppDataLocation, themeFolder,
                                               QStandardPaths::LocateDirectory);
    }

    const QString curPath = qApp->applicationDirPath();
    themeFolders += curPath % QDir::separator() % themeFolder % QDir::separator();

    return themeFolders;
}
