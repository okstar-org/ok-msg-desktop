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
#ifndef INSTALLOSX_H
#define INSTALLOSX_H

#include <QtCore/qsystemdetection.h>

#ifndef Q_OS_OSX
#error "This file is only meant to be compiled for Mac OSX targets"
#endif

namespace osx {
static constexpr int EXIT_UPDATE_MACX =
        218;  // We track our state using unique exit codes when debugging
static constexpr int EXIT_UPDATE_MACX_FAIL = 216;

void moveToAppFolder();
void migrateProfiles();
}  // namespace osx

#endif  // INSTALLOSX_H
