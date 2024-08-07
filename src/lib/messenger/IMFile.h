/*
 * Copyright (c) 2022 船山信息 chuanshaninfo.com
 * The project is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PubL v2 for more details.
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

private:
    void rejectFileRequest(const QString& friendId, const QString& sId);
    void acceptFileRequest(const QString& friendId, const File& file);
    void finishFileRequest(const QString& friendId, const QString& sId);
    void finishFileTransfer(const QString& friendId, const QString& sId);

    bool sendFile(const QString& friendId, const File& file);
    bool sendFileToResource(const JID& friendId, const File& file);

    IM* im;
    IMJingle* jingle;
    std::vector<FileHandler*> fileHandlers;

    // file
    QList<File> m_waitSendFiles;
    // k: file.id
    QMap<QString, IMFileTask*> m_fileSenderMap;

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
