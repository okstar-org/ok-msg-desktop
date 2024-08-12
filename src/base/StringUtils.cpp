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
#include "StringUtils.h"

#include <algorithm>
#include <cstring>
#include <vector>

namespace olc {

namespace base {

void str_tolower(std::string& s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
}

bool str_equals(std::string s1, std::string s2, bool case_sensitivity) {
    if (!case_sensitivity) {
        str_tolower(s1);
        str_tolower(s2);
    }
    return s1 == s2;
}

void str_join(std::vector<std::string>& v, std::string& out, const std::string& dot) {
    for (std::size_t i = 0; i < v.size(); i++) {
        out += v.at(i);
        if (i < v.size() - 1) {
            out += dot;
        }
    }
}

}  // namespace base

}  // namespace olc
