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

#include <QPixmap>
#include <QString>

#ifndef STATUS_H
#define STATUS_H

namespace module::im {

enum class AvatarSet { None, DefaultSet, UserSet };
// Status is weird, but Status is a fitting name for both the namespace and enum class..
enum class Status { None = -1, Online = 0, Away, Busy, Offline, Blocked };
QString getIconPath(Status status, bool event = false);
QString getTitle(Status status);
QString getAssetSuffix(Status status);
bool isOnline(Status status);

}  // namespace module::im

#endif  // STATUS_H
