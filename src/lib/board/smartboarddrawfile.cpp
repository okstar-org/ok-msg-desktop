
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

#include "smartboarddrawfile.h"

namespace lib::board {
DrawFile::DrawFile() {}

DrawFile::DrawFile(const std::string& id, const std::string& name) {
    _id = id;
    _name = name;
};

const size_t DrawFile::size() const {
    return _size;
};
void DrawFile::setSize(size_t size) {
    _size = size;
}

const std::string& DrawFile::name() const {
    return _name;
};
void DrawFile::setName(const std::string& name) {
    _name = name;
};

const std::string& DrawFile::url() const {
    return _url;
};
void DrawFile::setUrl(const std::string& url) {
    _url = url;
};

const std::string& DrawFile::md5() const {
    return _md5;
};
void DrawFile::setMd5(const std::string& md5) {
    _md5 = md5;
};

const std::string& DrawFile::token() const {
    return _url;
};

void DrawFile::setToken(const std::string& token) {
    _token = token;
}
const std::string& DrawFile::contentType() const {
    return _contentType;
};
void DrawFile::setContentType(const std::string& contentType) {
    _contentType = contentType;
}

}  // namespace lib::board