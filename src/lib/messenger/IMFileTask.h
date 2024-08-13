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

//
// Created by gaojie on 24-5-28.
//

#ifndef OKMSG_PROJECT_IMFILETASK_H
#define OKMSG_PROJECT_IMFILETASK_H

#include <QThread>
#include <memory>
#include "IMFile.h"

#include <bytestreamdatahandler.h>
#include <inbandbytestream.h>
#include <jid.h>

class QFile;

namespace lib::messenger {

class IM;
struct File;

class IMFileTask : public QThread, public gloox::BytestreamDataHandler {
    Q_OBJECT
public:
    IMFileTask(const QString& friendId,  //
               const File* file,         //
               IMFile* sender);          //

    ~IMFileTask();

    void run() override;

    void abort();

    void handleBytestreamData(gloox::Bytestream* bs, const std::string& data) override;

    void handleBytestreamDataAck(gloox::Bytestream* bs) override;

    void handleBytestreamError(gloox::Bytestream* bs, const gloox::IQ& iq) override;

    void handleBytestreamOpen(gloox::Bytestream* bs) override;

    void handleBytestreamClose(gloox::Bytestream* bs) override;

    void forceQuit();

    bool sendFinished() const;
    bool ackFinished() const;

private:
    QString m_friendId;
    const File* m_file;
    IMFile* m_im;
    int m_buf;
    int m_seq;
    quint64 m_sentBytes;
    int m_ack_seq;

    std::unique_ptr<gloox::InBandBytestream> m_ibb;
    std::unique_ptr<QFile> qFile;
    gloox::Bytestream* m_byteStream;

public:
signals:
    void fileSent(const QString& m_friendId, const File& m_file);
    void fileError(const QString& m_friendId, const File& m_file, int m_sentBytes);
    void fileAbort(const QString& m_friendId, const File& m_file, int m_sentBytes);
    void fileSending(
            const QString& m_friendId, const File& m_file, int m_seq, int m_sentBytes, bool end);
};

}  // namespace lib::messenger

#endif  // OKMSG_PROJECT_IMFILETASK_H
