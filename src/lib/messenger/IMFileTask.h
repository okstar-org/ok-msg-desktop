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

#pragma once

#include <memory>
#include "IMFile.h"

#include <bytestreamdatahandler.h>
#include <inbandbytestream.h>
#include <jid.h>
#include <QFile>
#include <QThread>

namespace lib::messenger {

class IM;
struct File;

class IMFileTask : public QThread, public gloox::BytestreamDataHandler {
public:
    IMFileTask(IM* m_im,
               const std::string& sId,
               const std::string& friendId,  //
               const File* file);            //

    ~IMFileTask() override;

    void run() override;

    void handleBytestreamData(gloox::Bytestream* bs, const std::string& data) override;

    void handleBytestreamDataAck(gloox::Bytestream* bs) override;

    void handleBytestreamError(gloox::Bytestream* bs, const gloox::IQ& iq) override;

    void handleBytestreamOpen(gloox::Bytestream* bs) override;

    void handleBytestreamClose(gloox::Bytestream* bs) override;

    void forceQuit();

    bool sendFinished() const;
    bool ackFinished() const;

private:
    IM* m_im;
    std::string m_sId;
    std::string m_friendId;
    const File* m_file;
    int m_buf;
    int m_seq;
    int m_ack_seq;
    uint64_t m_sentBytes;

    std::unique_ptr<gloox::InBandBytestream> m_ibb;
    std::unique_ptr<QFile> qFile;
    gloox::Bytestream* m_byteStream;
};

}  // namespace lib::messenger
