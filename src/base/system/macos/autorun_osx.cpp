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

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QStandardPaths>
#include "base/autorun.h"

int state;

bool Platform::setAutorun(bool on) {
    QString qtoxPlist =
            QDir::cleanPath(QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +
                            QDir::separator() + "Library" + QDir::separator() + "LaunchAgents" +
                            QDir::separator() + "chat.tox.qtox.autorun.plist");
    QString qtoxDir =
            QDir::cleanPath(QCoreApplication::applicationDirPath() + QDir::separator() + "qtox");
    QSettings autoRun(qtoxPlist, QSettings::NativeFormat);
    autoRun.setValue("Label", "chat.tox.qtox.autorun");
    autoRun.setValue("Program", qtoxDir);

    state = on;
    autoRun.setValue("RunAtLoad", state);
    return true;
}

bool Platform::getAutorun() { return state; }
