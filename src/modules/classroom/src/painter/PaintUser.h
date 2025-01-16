

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

#include <memory>
#include <vector>

struct SPaintUserInfoData {
    SPaintUserInfoData()
            : listenTcpPort(0)
            , superPeerCandidate(false)
            , screenRecording(false)
            , screenStreaming(false)
            , screenStreamingReceiver(false) {}

    std::string channel;
    std::string userId;
    std::string nickName;
    std::uint16_t listenTcpPort;
    std::string viewIp;
    std::string localIp;
    bool superPeerCandidate;
    bool screenRecording;
    bool screenStreaming;
    bool screenStreamingReceiver;
};

class CPaintUser;

typedef std::vector<std::shared_ptr<CPaintUser> > USER_LIST;

class CPaintUser {
public:
    CPaintUser(bool myself) : mySelfFlag_(myself), screenStreamListenPort_(0) {}
    CPaintUser(void) : mySelfFlag_(false) {}
    ~CPaintUser(void) {}

    // session id is only used for "always p2p mode"
    void setSessionId(int sessionId) {
        sessionId_ = sessionId;
    }
    int sessionId(void) {
        return sessionId_;
    }

    void setMyself(void) {
        mySelfFlag_ = true;
    }
    void setData(const struct SPaintUserInfoData& info) {
        data_ = info;
    }
    void setNickName(const std::string& nick) {
        data_.nickName = nick;
    }
    void setChannel(const std::string& channel) {
        data_.channel = channel;
    }
    void setListenTcpPort(std::uint16_t port) {
        data_.listenTcpPort = port;
    }
    void setSuperPeerCandidate(bool enable = true) {
        data_.superPeerCandidate = enable;
    }
    void setLocalIPAddress(const std::string& ip) {
        data_.localIp = ip;
    }
    void setViewIPAddress(const std::string& ip) {
        data_.viewIp = ip;
    }
    void setScreenRecording(bool status) {
        data_.screenRecording = status;
    }
    void setScreenStreaming(bool status) {
        data_.screenStreaming = status;
    }
    void setScreenStreamingReceiver(bool status) {
        data_.screenStreamingReceiver = status;
    }
    void setScreenStreamListenPort(std::uint16_t port) {
        screenStreamListenPort_ = port;
    }

    bool isMyself(void) {
        return mySelfFlag_;
    }
    const struct SPaintUserInfoData& data(void) {
        return data_;
    }
    bool isSuperPeerCandidate(void) {
        return data_.superPeerCandidate;
    }
    const std::string& localIPAddress(void) {
        return data_.localIp;
    }
    const std::string& viewIPAddress(void) {
        return data_.viewIp;
    }
    const std::string& channel(void) {
        return data_.channel;
    }
    const std::string& userId(void) {
        return data_.userId;
    }
    const std::string& nickName(void) {
        return data_.nickName;
    }
    std::uint16_t listenTcpPort(void) {
        return data_.listenTcpPort;
    }
    bool isScreenRecording(void) {
        return data_.screenRecording;
    }
    bool isScreenStreaming(void) {
        return data_.screenStreaming;
    }
    bool isScreenStreamingReceiver(void) {
        return data_.screenStreamingReceiver;
    }

    bool isAvailableRecvScreenStream(void) {
        return screenStreamListenPort_ != 0;
    }
    std::uint16_t screenStreamListenPort(void) {
        return screenStreamListenPort_;
    }

    std::string serialize(void) {
        std::string body;

        return body;
    }

    bool deserialize(const std::string& data, int* readPos = nullptr) {
        return true;
    }

private:
    bool mySelfFlag_;
    std::uint16_t screenStreamListenPort_;
    int sessionId_;
    SPaintUserInfoData data_;
};
