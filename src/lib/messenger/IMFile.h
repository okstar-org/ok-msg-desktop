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

#include <QFile>
#include <QThread>
#include "IM.h"
#include "IMJingle.h"

namespace gloox {
class JID;
class BytestreamDataHandler;
}  // namespace gloox

namespace lib::messenger {

class IMFileTask;
class IM;

/**
 * 传输文件会话，一次会话代表一次文件传输请求
 */
class IMFileSession : public QObject, public IMJingleSession {
    Q_OBJECT
public:
    IMFileSession(const QString& sId,
                  gloox::Jingle::Session* session_,
                  const IMPeerId& peerId,
                  IMFile* sender,
                  File* file);
    ~IMFileSession() override;
    void start() override;
    void stop() override;

    File* getFile() {
        return file;
    }
    gloox::Jingle::Session* getJingleSession() {
        return session;
    }

private:
    QString sId;
    IMFile* sender;
    File* file;

    // 对方
    IMPeerId target;

    // 自己
    QString self;

    // session
    gloox::Jingle::Session* session;

    std::unique_ptr<IMFileTask> task;
};

class IMFile : public IMJingle, public IMSessionHandler {
    Q_OBJECT
public:
    explicit IMFile(IM* im, QObject* parent = nullptr);
    ~IMFile() override;

    void addFile(const File& f);
    void addFileHandler(FileHandler*);

    /**
     * File
     */
    void fileRejectRequest(const QString& friendId, const File& file);
    void fileAcceptRequest(const QString& friendId, const File& file);
    void fileFinishRequest(const QString& friendId, const QString& sId);
    void fileFinishTransfer(QString friendId, const QString& sId);
    void fileCancel(QString fileId);
    bool fileSendToFriend(const QString& f, const File& file);

    bool handleIq(const gloox::IQ& iq) override;

    /**
     * 启动文件发送任务
     * @param session
     * @param file
     */
    void doStartFileSendTask(const gloox::Jingle::Session* session, const File& file);

    /**
     * 停止文件发送任务
     * @param session
     * @param file
     */
    void doStopFileSendTask(const gloox::Jingle::Session* session, const File& file);

    std::vector<FileHandler*> getHandlers() {
        return fileHandlers;
    }

    IMFileSession* findSession(const QString& sId) {
        return m_fileSessionMap.value(sId);
    }

    void clearSessionInfo(const QString& sId) override;

protected:
    void handleJingleMessage(const IMPeerId& peerId,
                             const gloox::Jingle::JingleMessage* jm) override {
        qWarning() << "Unable to handle messages from:" << peerId.toString();
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

private:
    void rejectFileRequest(const QString& friendId, const QString& sId);
    void acceptFileRequest(const QString& friendId, const File& file);
    void finishFileRequest(const QString& friendId, const QString& sId);
    void finishFileTransfer(const QString& friendId, const QString& sId);

    bool sendFile(const QString& friendId, const File& file);
    bool sendFileToResource(const gloox::JID& friendId, const File& file);

    std::vector<FileHandler*> fileHandlers;

    // file
    QList<File> m_waitSendFiles;

    // 文件传输会话（key = session.dataId=file.id）
    QMap<QString, IMFileSession*> m_fileSessionMap;

signals:
    void sendFileInfo(const QString& friendId, const File& file, int m_seq, int m_sentBytes,
                      bool end);

    void sendFileAbort(const QString& friendId, const File& file, int m_sentBytes);
    void sendFileError(const QString& friendId, const File& file, int m_sentBytes);

    void receiveFileRequest(const QString& friendId, const File& file);

    void receiveFileChunk(const IMContactId friendId, QString sId, int seq,
                          const std::string chunk);

    void receiveFileFinished(const IMContactId friendId, QString sId);

public slots:
    void onImStartedFile();
};

}  // namespace lib::messenger

#endif
