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

#ifndef TOXCLIENTSTANDARDS_H
#define TOXCLIENTSTANDARDS_H

#include <cstdint>

namespace ToxClientStandards {
// From TCS 2.2.4, max valid avatar size is 64KiB
constexpr static uint64_t MaxAvatarSize = 64 * 1024;
constexpr bool IsValidAvatarSize(uint64_t fileSize) { return fileSize <= MaxAvatarSize; }
}  // namespace ToxClientStandards

#endif  // TOXCLIENTSTANDARDS_H
