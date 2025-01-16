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

#include <string>
#include "src/painter/PaintUser.h"

namespace SystemPacketBuilder {
class CChatMessage {
public:
    static std::string make(const std::string& id,
                            const std::string& nick,
                            const std::string& msg) {
        std::string s;
        return s;
    }

    static bool parse(const std::string& body,
                      std::string& id,
                      std::string& nick,
                      std::string& msg) {
        return true;
    }
};

class CVersionInfo {
public:
    static std::string make(const std::string& version, const std::string& protVersion) {
        std::string s;
        return s;
    }

    static bool parse(const std::string& body, std::string& version, std::string& protVersion) {
        return false;
    }
};

class CChangeNickName {
public:
    static std::string make(const std::string& userid, const std::string& nickName) {
        std::string s;
        return s;
    }

    static bool parse(const std::string& body, std::string& userid, std::string& nickName) {
        return false;
    }
};

class CChangeSuperPeer {
public:
    static bool parse(const std::string& body, std::string& userid) {
        return false;
    }
};

class CJoinToServer {
public:
    static std::string make(std::shared_ptr<CPaintUser> user) {
        return "";
    }

    static std::shared_ptr<CPaintUser> parse(const std::string& body) {
        return std::shared_ptr<CPaintUser>();
    }
};

class CJoinerToSuperPeer {
public:
    static std::string make(std::shared_ptr<CPaintUser> user) {
        return "";
    }

    static std::shared_ptr<CPaintUser> parse(const std::string& body) {
        return std::shared_ptr<CPaintUser>();
    }
};

class CSyncRequest {
public:
    static std::string make(void) {
        return "";
    }

    static bool parse(const std::string& body, std::string& channel, std::string& target) {
        return false;
    }
};

class CSyncStart {
public:
    static std::string make(const std::string& channel,
                            const std::string& fromId,
                            const std::string& toId) {
        return "";
    }

    static bool parse(const std::string& body, std::string& channel) {
        return false;
    }
};

class CSyncComplete {
public:
    static std::string make(const std::string& targetId) {
        return "";
    }

    static bool parse(const std::string& body) {
        return false;
    }
};

class CTcpSyn {
public:
    static bool parse(const std::string& body) {
        return false;
    }
};

class CTcpAck {
public:
    static std::string make(void) {
        return "";
    }
};

class CResponseJoin {
public:
    static bool parse(const std::string& body,
                      std::string& channel,
                      bool& firstFlag,
                      USER_LIST& list,
                      std::string& superPeerId) {
        return false;
    }
};

class CLeftUser {
public:
    static std::string make(const std::string& channel, const std::string& userId) {
        return "";
    }

    static bool parse(const std::string& body, std::string& channel, std::string& userId) {
        return false;
    }
};

class CHistoryUserList {
public:
    static std::string make(USER_LIST list) {
        return "";
    }

    static USER_LIST parse(const std::string& body) {
        return USER_LIST();
    }
};

};  // namespace SystemPacketBuilder
