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

#include "ok_rtc_defs.h"

namespace lib::ortc {

bool OJingleContentAv::isValid() { return !contents.empty(); }

bool OJingleContentAv::isVideo() const {
    for (auto& c : contents)
        if (c.second.rtp.media == Media::video) return true;
    return false;
}

bool OJingleContentFile::isValid() {
    for (auto& c : contents)
        if (!c.name.empty() && c.size > 0) return true;
    return false;
}

}  // namespace lib::ortc
