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
class IMFileSession : public QObject, IMJingSession {
    Q_OBJECT
public:
    IMFileSession(const QString& sId,
                  Jingle::Session* session_,
                  const IMPeerId& peerId,
                  IMFile* sender,
                  File* file);
    void start();
    void stop();

    Session* getJingleSession() { return session; }

protected:
private:
    QString sId;
    IMFile* sender;
    File* file;

    // 对方
    IMPeerId target;

    // 自己
    QString self;

    // session
    Session* session;

    std::unique_ptr<IMFileTask> task;
};

class IMFile : public IMJingle {
    Q_OBJECT
public:
    IMFile(IM* im, QObject* parent = nullptr);
    ~IMFile();

    void addFile(const File& f);
    void addFileHandler(FileHandler*);

    /**
     * File
     */
    void fileRejectRequest(QString friendId, const File& file);
    void fileAcceptRequest(QString friendId, const File& file);
    void fileFinishRequest(QString friendId, const QString& sId);
    void fileFinishTransfer(QString friendId, const QString& sId);
    void fileCancel(QString fileId);
    bool fileSendToFriend(const QString& f, const File& file);

    // 收到对方发起
    void sessionOnInitiate(const QString& sId,
                           Jingle::Session* session,
                           const Jingle::Session::Jingle* jingle,
                           const IMPeerId& peerId);
    // 对方接受
    void sessionOnAccept(const QString& sId,
                         Jingle::Session* session,
                         const IMPeerId& peerId) override;
    // 对方终止
    void sessionOnTerminate(const QString& sId, const IMPeerId& peerId) override;

    bool handleIq(const IQ& iq) override;

    /**
     * 启动文件发送任务
     * @param session
     * @param file
     */
    void doStartFileSendTask(const Jingle::Session* session, const File& file);

    /**
     * 停止文件发送任务
     * @param session
     * @param file
     */
    void doStopFileSendTask(const Jingle::Session* session, const File& file);

    std::vector<FileHandler*> getHandlers() { return fileHandlers; }

private:
    void rejectFileRequest(const QString& friendId, const QString& sId);
    void acceptFileRequest(const QString& friendId, const File& file);
    void finishFileRequest(const QString& friendId, const QString& sId);
    void finishFileTransfer(const QString& friendId, const QString& sId);

    bool sendFile(const QString& friendId, const File& file);
    bool sendFileToResource(const JID& friendId, const File& file);

    std::vector<FileHandler*> fileHandlers;

    // file
    QList<File> m_waitSendFiles;

    // k: file.id
    //    QMap<QString, IMFileTask*> m_fileSenderMap;

    // 文件传输会话（key= session.dataId=file.id）
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
};

}  // namespace lib::messenger

#endif
