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

#ifndef PATHS_H
#define PATHS_H

#include <QString>
#include <QStringList>

class Paths {
public:
    enum class Portable {
        Auto,       /** Auto detect if portable or non-portable */
        Portable,   /** Force portable mode */
        NonPortable /** Force non-portable mode */
    };

    static Paths* makePaths(Portable mode = Portable::Auto);

    bool isPortable() const;
    QString getGlobalSettingsPath() const;
    QString getProfilesDir() const;
    QString getToxSaveDir() const;
    QString getAvatarsDir() const;
    QString getTransfersDir() const;
    QStringList getThemeDirs() const;
    QString getScreenshotsDir() const;

private:
    Paths(const QString& basePath, bool portable);

private:
    QString basePath{};
    bool portable = false;
};

#endif  // PATHS_H
