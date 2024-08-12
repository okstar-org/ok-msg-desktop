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

#include "base/autorun.h"

#include <QApplication>
#include <QDir>
#include <QProcessEnvironment>
#include "base/r.h"

namespace Platform {
QString getAutostartDirPath() {
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString config = env.value("XDG_CONFIG_HOME");
    if (config.isEmpty()) config = QDir::homePath() + "/" + ".config";
    return config + "/autostart";
}

QString getAutostartFilePath(QString dir) { return dir + "/" APPLICATION_NAME + ".desktop"; }

inline QString currentCommandLine() { return QApplication::applicationFilePath(); }
}  // namespace Platform

bool Platform::setAutorun(bool on) {
    QString dirPath = getAutostartDirPath();
    QFile desktop(getAutostartFilePath(dirPath));
    if (on) {
        if (!QDir().mkpath(dirPath) || !desktop.open(QFile::WriteOnly | QFile::Truncate))
            return false;
        desktop.write("[Desktop Entry]\n");
        desktop.write("Type=Application\n");
        desktop.write("Name=" APPLICATION_EXE_NAME "\n");
        desktop.write("Exec=\"");
        desktop.write(currentCommandLine().toUtf8());
        desktop.write("\"\n");
        desktop.close();
        return true;
    } else
        return desktop.remove();
}

bool Platform::getAutorun() { return QFile(getAutostartFilePath(getAutostartDirPath())).exists(); }
