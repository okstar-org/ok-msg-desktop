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

#pragma once

#include "Defines.h"

namespace lib::board {

class DrawFile : public DrawItem {
private:
    size_t _size;
    std::string _name;
    std::string _url;
    std::string _md5;
    std::string _token;
    std::string _contentType;

public:
    DrawFile();
    DrawFile(const std::string& id, const std::string& name);

    const size_t size() const;
    void setSize(size_t size);

    const std::string& name() const;
    void setName(const std::string&);

    const std::string& url() const;
    void setUrl(const std::string&);

    const std::string& md5() const;
    void setMd5(const std::string&);

    const std::string& token() const;
    void setToken(const std::string&);

    const std::string& contentType() const;
    void setContentType(const std::string&);
};

}  // namespace lib::board