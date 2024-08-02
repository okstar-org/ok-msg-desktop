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
#include "lib/messenger/messenger.h"

namespace gloox {
class JID;
class BytestreamDataHandler;
}  // namespace gloox

namespace lib {
namespace messenger {

class IM;

// 不要修改顺序和值
enum class FileStatus {
    INITIALIZING = 0,
    PAUSED = 1,
    TRANSMITTING = 2,
    BROKEN = 3,
    CANCELED = 4,
    FINISHED = 5,
};

// 不要修改顺序和值
enum class FileDirection {
    SENDING = 0,
    RECEIVING = 1,
};

enum class FileControl { RESUME, PAUSE, CANCEL };

struct FileTxIBB {
    QString sid;
    int blockSize;
};

struct File {
public:
    // id(file id = ibb id) 和 sId(session id)
    QString id;
    QString sId;
    QString name;
    QString path;
    quint64 size;
    FileStatus status;
    FileDirection direction;
    FileTxIBB txIbb;
    [[__nodiscard__]] QString toString() const;
    friend QDebug& operator<<(QDebug& debug, const File& f);
};

class FileHandler {
public:
    virtual void onFileRequest(const QString& friendId, const File& file) = 0;
    virtual void onFileRecvChunk(const QString& friendId, const QString& fileId, int seq,
                                 const std::string& chunk) = 0;
    virtual void onFileRecvFinished(const QString& friendId, const QString& fileId) = 0;
    virtual void onFileSendInfo(const QString& friendId, const File& file, int m_seq,
                                int m_sentBytes, bool end) = 0;
    virtual void onFileSendAbort(const QString& friendId, const File& file, int m_sentBytes) = 0;
    virtual void onFileSendError(const QString& friendId, const File& file, int m_sentBytes) = 0;
};

class IMFile : public QObject {
public:
    IMFile(QObject* parent = nullptr);
    ~IMFile();
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

private:
    IMJingle* jingle;
    std::vector<FileHandler*> fileHandlers;
};

}  // namespace messenger
}  // namespace lib

using ToxFile1 = lib::messenger::IMFile;

#endif  // OKEDU_CLASSROOM_DESKTOP_IMFILE_H
