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

#include "IMFileTask.h"
#include "IM.h"
#include "IMFile.h"

#include <jid.h>
#include <fstream>
#include <iostream>

namespace lib::messenger {

constexpr int BUF_SIZE = 100 * 1024;  // 100k

IMFileTask::IMFileTask(const std::string& friendId, const File* file, IMFile* im)
        : m_friendId(friendId)
        , m_file(file)
        , m_im(im)
        , m_byteStream(nullptr)
        , m_buf{BUF_SIZE}
        , m_seq{0}
        , m_sentBytes{0}
        , m_ack_seq{0} {}

IMFileTask::~IMFileTask() {}

void IMFileTask::run() {
    /**
     * 1、创建流通道
     * https://xmpp.org/extensions/xep-0047.html#create
     *
     */
    auto* im = m_im->getIM();
    auto client = im->getClient();
    //  client->registerStanzaExtension(new InBandBytestream::IBB);

    auto iqId = client->getID();

    m_ibb = std::make_unique<gloox::InBandBytestream>(client,                    //
                                                      client->logInstance(),     //
                                                      im->self(),                //
                                                      gloox::JID((m_friendId)),  //
                                                      (m_file->id));
    m_ibb->registerBytestreamDataHandler(this);
    m_ibb->setBlockSize(m_buf);

    bool c = m_ibb->connect();

    std::fstream file(qFile, std::ios::in | std::ios::binary);
    int waitingSecs = 0;

    for (;;) {
        if (!m_byteStream) {
            sleep(1);
            if (waitingSecs++ >= 60) {
                std::cerr << "Timeout to wait stream open." << waitingSecs << std::endl;
                break;
            }
            continue;
        }

        std::vector<char> buf(m_buf);
        file.read(buf.data(), m_buf);

        if (buf.empty()) {
            break;
        }

        std::string str(buf.begin(), buf.end());
        bool b = m_byteStream->send(str);
        if (!b) {
            std::cerr << "send error" << std::endl;
            handler->fileError(m_friendId, *m_file, m_sentBytes);
            break;
        }

        m_seq += 1;
        m_sentBytes += buf.size();
        handler->fileSending(m_friendId, *m_file, m_ack_seq, m_ack_seq * m_buf, false);
    }

    if (m_byteStream) {
        m_byteStream->close();
        handler->fileSending(m_friendId, *m_file, m_ack_seq, m_sentBytes, true);
    }

    std::cout << "finished." << std::endl;
}

// void IMFileTask::abort() {
//     handler->fileAbort(m_friendId, *m_file, m_sentBytes);
// }

void IMFileTask::handleBytestreamOpen(gloox::Bytestream* bs) {
    //    qDebug() << __func__ << ("file") << qFile->fileName();
    //    if (!qFile->open(QIODevice::ReadOnly)) {
    //        return;
    //    }
    m_byteStream = bs;
}

void IMFileTask::handleBytestreamClose(gloox::Bytestream* bs) {
    //    qDebug() << __func__ << "closed:" << (qstring(bs->sid()));
    handler->fileSent(m_friendId, *m_file);
}

void IMFileTask::handleBytestreamData(gloox::Bytestream* bs, const std::string& data) {
    //    qDebug() << __func__ << "data:" << qstring(bs->sid());
}

void IMFileTask::handleBytestreamDataAck(gloox::Bytestream* bs) {
    //    qDebug() << __func__ << "acked:" << qstring(bs->sid());
    m_ack_seq += 1;
    // 考虑性能关系暂时不处理实时反馈ack
    //   if(m_ack_seq < m_seq){
    //     emit fileSending(m_friendId, m_file, m_ack_seq, m_ack_seq*m_buf,
    //     false);
    //   } else{
    //     emit fileSending(m_friendId, m_file, m_ack_seq, m_sentBytes, true);
    //   }
}

void IMFileTask::handleBytestreamError(gloox::Bytestream* bs, const gloox::IQ& iq) {
    //    qDebug() << __func__ << qstring(bs->sid());
    handler->fileError(m_friendId, *m_file, m_sentBytes);
}

bool IMFileTask::sendFinished() const {
    return m_file->size == m_sentBytes;
}
bool IMFileTask::ackFinished() const {
    return m_seq > 0 && m_seq == m_ack_seq;
}

void IMFileTask::forceQuit() {
    //    qDebug() << __func__ << "...";
    //    quit();
    //    wait();
    //    qDebug() << __func__ << "completed.";
}

}  // namespace lib::messenger
