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

#include "IMFile.h"
#include "base/logs.h"
#include <QEventLoop>
#include <gloox/src/bytestream.h>
#include <gloox/src/inbandbytestream.h>
#include <utility>

namespace lib {
namespace messenger {
IMFile::IMFile(const JID &friendId, FileHandler::File file, const IM *im)
    : m_friendId(friendId), m_file(std::move(file)), m_im(im),
      m_byteStream(nullptr) {
  DEBUG_LOG(("Create for:%1").arg(m_file.id));
}

IMFile::~IMFile() { DEBUG_LOG(("Destroyed for:%1").arg(m_file.id)); }

void IMFile::run() {

  QThread::currentThread()->setObjectName(
      tr("FileSender-%1-%2")
          .arg(qstring(m_friendId.username()))
          .arg(m_file.id));

  DEBUG_LOG(("Start file:%1").arg(m_file.id));

  /**
   * 1、创建流通道
   * https://xmpp.org/extensions/xep-0047.html#create
   *
   */
  auto client = m_im->getClient();
  //  client->registerStanzaExtension(new InBandBytestream::IBB);

  auto iqId = client->getID();

  m_ibb = std::make_unique<InBandBytestream>(client,
                                             client->logInstance(), //
                                             m_im->self(), m_friendId,
                                             stdstring(m_file.id));

  m_ibb->registerBytestreamDataHandler(this);
  m_ibb->setBlockSize(m_buf);

  bool c = m_ibb->connect();
  DEBUG_LOG(("IBBConnect=>%1").arg(c))
  qFile = std::make_unique<QFile>(m_file.path);

  //  QEventLoop loop;
  //  connect(this, &IMFile::fileSent, &loop, &QEventLoop::quit);
  //  loop.exec();

  for (;;) {

    if (!m_byteStream) {
      sleep(100);
      continue;
    }

    QByteArray buf = qFile->read(m_buf);
    if (buf.isEmpty()) {
      break;
    }
    bool b = m_byteStream->send(buf.toStdString());
    if (!b) {
      qWarning() << "send error";
      emit fileError(m_friendId, m_file, m_sentBytes);
      break;
    }
    m_seq += 1;
    m_sentBytes += buf.size();
    emit fileSending(m_friendId, m_file, m_ack_seq, m_ack_seq * m_buf, false);
  }

  m_byteStream->close();
  qFile->close();

  emit fileSending(m_friendId, m_file, m_ack_seq, m_sentBytes, true);

  DEBUG_LOG(("finished."))
}

void IMFile::abort() { emit fileAbort(m_friendId, m_file, m_sentBytes); }

void IMFile::handleBytestreamOpen(gloox::Bytestream *bs) {
  DEBUG_LOG(("file:%1").arg(qFile->fileName()))
  if (!qFile->open(QIODevice::ReadOnly)) {
    return;
  }
  m_byteStream = bs;
}

void IMFile::handleBytestreamClose(gloox::Bytestream *bs) {
  DEBUG_LOG(("closed:%1").arg(qstring(bs->sid())))
  emit fileSent(m_friendId, m_file);
}

void IMFile::handleBytestreamData(gloox::Bytestream *bs,
                                  const std::string &data) {
  DEBUG_LOG(("data:%1").arg(qstring(bs->sid())))
}

void IMFile::handleBytestreamDataAck(gloox::Bytestream *bs) {
  DEBUG_LOG(("acked:%1").arg(qstring(bs->sid())));
  m_ack_seq += 1;
  // TODO 考虑性能关系暂时不处理实时反馈ack
  //   if(m_ack_seq < m_seq){
  //     emit fileSending(m_friendId, m_file, m_ack_seq, m_ack_seq*m_buf,
  //     false);
  //   } else{
  //     emit fileSending(m_friendId, m_file, m_ack_seq, m_sentBytes, true);
  //   }
}

void IMFile::handleBytestreamError(gloox::Bytestream *bs, const gloox::IQ &iq) {
  DEBUG_LOG(("error:%1").arg(qstring(bs->sid())))
  fileError(m_friendId, m_file, m_sentBytes);
}

} // namespace messenger
} // namespace lib