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
#include "base/basic_types.h"

#include <jid.h>
#include <QDebug>
#include <QFile>

namespace lib::messenger {

constexpr int BUF_SIZE = 4 * 1024;  // 4k

IMFileTask::IMFileTask(IM* m_im,
                       const std::string& sId,
                       const std::string& friendId,
                       const File* file)
        : m_im(m_im)
        , m_sId(sId)
        , m_friendId(friendId)
        , m_file(file)
        , m_byteStream(nullptr)
        , m_buf{BUF_SIZE}
        , m_seq{0}
        , m_sentBytes{0}
        , m_ack_seq{0} {
    qDebug() << __func__;
}

IMFileTask::~IMFileTask() {
    qDebug() << __func__;
}

void IMFileTask::run() {
    qDebug() << __func__;
    /**
     * 1、创建流通道
     * https://xmpp.org/extensions/xep-0047.html#create
     *
     */

    auto client = m_im->getClient();
    //  client->registerStanzaExtension(new InBandBytestream::IBB);

    auto iqId = client->getID();

    m_ibb = std::make_unique<gloox::InBandBytestream>(client,                    //
                                                      client->logInstance(),     //
                                                      client->jid(),             //
                                                      gloox::JID((m_friendId)),  //
                                                      (m_file->id));
    m_ibb->registerBytestreamDataHandler(this);
    m_ibb->setBlockSize(m_buf);

    bool c = m_ibb->connect();

    int waitingSecs = 0;
    qFile = std::make_unique<QFile>(qstring(m_file->path));
    for (;;) {
        if (!m_byteStream) {
            sleep(1);
            if (waitingSecs++ >= 60) {
                qWarning() << "Timeout to wait stream open." << waitingSecs;
                break;
            }
            continue;
        }

        QByteArray buf = qFile->read(m_buf);
        if (buf.isEmpty()) {
            break;
        }
        bool b = m_byteStream->send(buf.toStdString());
        if (!b) {
            qWarning() << "send error!";
            for (auto handler : m_file->handlers) {
                handler->onFileSendError(m_sId, m_friendId, *m_file, m_sentBytes);
            }
            break;
        }

        m_seq += 1;
        m_sentBytes += buf.size();
        for (auto handler : m_file->handlers) {
            handler->onFileStreamData(m_sId, m_friendId, *m_file, {}, m_ack_seq, m_sentBytes);
        }
    }

    if (m_byteStream) {
        m_byteStream->close();
    }

    qFile->close();
    qDebug() << __func__ << "is finished.";
}

// void IMFileTask::abort() {
//     handler->fileAbort(m_friendId, *m_file, m_sentBytes);
// }

void IMFileTask::handleBytestreamOpen(gloox::Bytestream* bs) {
    qDebug() << __func__ << m_file->path.c_str();
    if (!qFile->open(QIODevice::ReadOnly)) {
        return;
    }
    m_byteStream = bs;
    for (auto handler : m_file->handlers) {
        handler->onFileStreamOpened(m_sId, m_friendId, *m_file);
    }
}

void IMFileTask::handleBytestreamClose(gloox::Bytestream* bs) {
    qDebug() << __func__ << "closed:" << ((bs->sid().c_str()));
    for (auto handler : m_file->handlers) {
        handler->onFileStreamClosed(m_sId, m_friendId, *m_file);
    }
    qFile->close();
}

void IMFileTask::handleBytestreamData(gloox::Bytestream* bs, const std::string& data) {
    qDebug() << __func__ << "chunk:" << m_seq << "sent bytes:" << m_sentBytes;
    for (auto handler : m_file->handlers) {
        handler->onFileStreamData(m_sId, m_friendId, *m_file, data, m_seq, m_sentBytes);
    }
}

void IMFileTask::handleBytestreamDataAck(gloox::Bytestream* bs) {
    qDebug() << __func__ << "acked:" << (bs->sid().c_str());
    m_ack_seq += 1;
    for (auto handler : m_file->handlers) {
        handler->onFileStreamDataAck(m_sId, m_friendId, *m_file, m_ack_seq);
    }
}

void IMFileTask::handleBytestreamError(gloox::Bytestream* bs, const gloox::IQ& iq) {
    qDebug() << __func__ << (bs->sid().c_str());
    for (auto handler : m_file->handlers) {
        handler->onFileStreamError(m_sId, m_friendId, *m_file, m_sentBytes);
    }
}

bool IMFileTask::sendFinished() const {
    return m_file->size == m_sentBytes;
}

bool IMFileTask::ackFinished() const {
    return m_seq > 0 && m_seq == m_ack_seq;
}

void IMFileTask::forceQuit() {
    qDebug() << __func__ << "...";
    quit();
    wait();
    qDebug() << __func__ << "completed.";
}

}  // namespace lib::messenger
