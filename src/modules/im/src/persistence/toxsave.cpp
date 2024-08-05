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

#include "toxsave.h"
#include <QCoreApplication>
#include "src/widget/gui.h"
#include "src/widget/tool/profileimporter.h"

bool toxSaveEventHandler(const QByteArray& eventData) {
    if (!eventData.endsWith(".tox")) {
        return false;
    }

    handleToxSave(eventData);
    return true;
}

/**
 * @brief Import new profile.
 * @note Will wait until the core is ready first.
 * @param path Path to .tox file.
 * @return True if import success, false, otherwise.
 */
bool handleToxSave(const QString& path) {
    ProfileImporter importer(GUI::getMainWidget());
    return importer.importProfile(path);
}
