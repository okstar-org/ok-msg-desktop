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
#ifndef IMFILE_H
#define IMFILE_H

#include "IM.h"
#include "IMJingle.h"

namespace gloox {
class JID;
class BytestreamDataHandler;
}  // namespace gloox

namespace lib::messenger {

class IM;
class IMFileTask;

/**
 * 传输文件会话，一次会话代表一次文件传输请求
 */
class IMFileSession : public IMJingleSession {
public:
    IMFileSession(const std::string& sId,
                  gloox::Jingle::Session* session_,
                  const IMPeerId& peerId,
                  IMFile* sender,
                  File* file);
    ~IMFileSession();
    void start() override;
    void stop() override;

    File* getFile() {
        return file;
    }
    gloox::Jingle::Session* getJingleSession() {
        return session;
    }

private:
    std::string sId;
    IMFile* sender;
    File* file;

    // 对方
    IMPeerId target;

    // 自己
    std::string self;

    // session
    gloox::Jingle::Session* session;

    std::unique_ptr<IMFileTask> task;
};

class IMFile : public IMJingle, public IMHandler, public IMSessionHandler {
public:
    explicit IMFile(IM* im);
    ~IMFile() override;

    void addFile(const File& f);

    /**
     * File
     */
    void fileRejectRequest(const std::string& friendId, const File& file);
    void fileAcceptRequest(const std::string& friendId, const File& file);
    void fileFinishRequest(const std::string& friendId, const std::string& sId);
    void fileFinishTransfer(std::string friendId, const std::string& sId);
    void fileCancel(std::string fileId);

    bool fileSendToFriend(const std::string& friendId, const File& file);
    bool sendFile(const std::string& friendId, const File& file);
    bool sendFileToResource(const gloox::JID& peerId, const File& file);

    bool handleIq(const gloox::IQ& iq) override;

    std::vector<FileHandler*> getHandlers();

    void addHandler(FileHandler* h);

    IMFileSession* findSession(const std::string& sId) {
        auto it = m_fileSessionMap.find(sId);
        return it == m_fileSessionMap.end() ? nullptr : it->second;
    }

    void clearSessionInfo(const std::string& sId) override;

protected:
    void handleJingleMessage(const IMPeerId& peerId,
                             const gloox::Jingle::JingleMessage* jm) override {
    }

    bool doSessionInitiate(gloox::Jingle::Session* session,        //
                           const gloox::Jingle::Session::Jingle*,  //
                           const IMPeerId&) override;

    bool doSessionTerminate(gloox::Jingle::Session* session,        //
                            const gloox::Jingle::Session::Jingle*,  //
                            const IMPeerId&) override;

    bool doSessionAccept(gloox::Jingle::Session* session,        //
                         const gloox::Jingle::Session::Jingle*,  //
                         const IMPeerId&) override;

    bool doSessionInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;

    bool doContentAdd(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;

    bool doContentRemove(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;

    bool doContentModify(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;

    bool doContentAccept(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;

    bool doContentReject(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;

    bool doTransportInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;

    bool doTransportAccept(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;

    bool doTransportReject(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;

    bool doTransportReplace(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;

    bool doSecurityInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;

    bool doDescriptionInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;

    bool doSourceAdd(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;

    bool doInvalidAction(const gloox::Jingle::Session::Jingle*, const IMPeerId&) override;

    /**
     * IMHandler
     */
    void onConnecting() override;
    void onConnected() override;
    void onDisconnected(int) override;
    void onStarted() override;
    void onStopped() override;

private:
    void rejectFileRequest(const std::string& friendId, const std::string& sId);
    void acceptFileRequest(const std::string& friendId, const File& file);
    void finishFileRequest(const std::string& friendId, const std::string& sId);
    void finishFileTransfer(const std::string& friendId, const std::string& sId);

    std::vector<FileHandler*> fileHandlers;

    // file
    QList<File> m_waitSendFiles;

    // 文件传输会话（key = session.dataId=file.id）
    std::map<std::string, IMFileSession*> m_fileSessionMap;

};

}  // namespace lib::messenger

#endif
