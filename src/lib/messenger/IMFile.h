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

#include "IM.h"
#include "lib/messenger/messenger.h"
#include <QFile>
#include <QThread>
#include <bytestreamdatahandler.h>
#include <inbandbytestream.h>
#include <iqhandler.h>

namespace lib {
namespace messenger {

class IMFile : public QThread, public BytestreamDataHandler {
  Q_OBJECT
public:
  IMFile(const JID &friendId,           //
         File file,        //
         const IM *im); //

  virtual ~IMFile();

  void run() override;

  void abort();

  void handleBytestreamData(Bytestream *bs, const std::string &data) override;

  void handleBytestreamDataAck(Bytestream *bs) override;

  void handleBytestreamError(Bytestream *bs, const IQ &iq) override;

  void handleBytestreamOpen(Bytestream *bs) override;

  void handleBytestreamClose(Bytestream *bs) override;

  bool sendFinished() const { return m_file.size == m_sentBytes; }
  bool ackFinished() const { return m_seq > 0 && m_seq == m_ack_seq; }

private:
  JID m_friendId;
  File m_file;
  const IM *m_im;
  int m_buf;
  int m_seq;
  quint64 m_sentBytes;
  int m_ack_seq;

  std::unique_ptr<InBandBytestream> m_ibb;
  std::unique_ptr<QFile> qFile;
  gloox::Bytestream *m_byteStream;


public:
signals:
  void fileSent(const JID &m_friendId, const File &m_file);
  void fileError(const JID &m_friendId, const File &m_file,
                 int m_sentBytes);
  void fileAbort(const JID &m_friendId, const File &m_file,
                 int m_sentBytes);
  void fileSending(const JID &m_friendId, const File &m_file,
                   int m_seq, int m_sentBytes, bool end);
};

} // namespace messenger
} // namespace lib
#endif // OKEDU_CLASSROOM_DESKTOP_IMFILE_H
