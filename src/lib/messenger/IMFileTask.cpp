//
// Created by gaojie on 24-5-28.
//

#include "IMFileTask.h"
#include "IM.h"
#include "IMFile.h"

#include <jid.h>

namespace lib::messenger {

constexpr int BUF_SIZE = 100 * 1024; // 100k

IMFileTask::IMFileTask(const gloox::JID &friendId, const File *file, const IM *im)
    : m_friendId(friendId), m_file(file), m_im(im), m_byteStream(nullptr), m_buf{BUF_SIZE}, m_seq{0}, m_sentBytes{0}, m_ack_seq{0} {
  qDebug() << __func__ << "Create FileSender:" << m_file->id;
}

IMFileTask::~IMFileTask() { qDebug() << __func__ << "Destroyed FileSender:" << m_file->id; }

void IMFileTask::run() {

  QThread::currentThread()->setObjectName(tr("FileSender-%1-%2").arg(qstring(m_friendId.username())).arg(m_file->id));

  qDebug() << "Start file" << m_file->id;

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
                                             m_im->self(),          //
                                             m_friendId,            //
                                             stdstring(m_file->id));

  m_ibb->registerBytestreamDataHandler(this);
  m_ibb->setBlockSize(m_buf);

  bool c = m_ibb->connect();
  qDebug() << "IBBConnect=>" << c;
  qFile = std::make_unique<QFile>(m_file->path);

  //  QEventLoop loop;
  //  connect(this, &IMFileTask::fileSent, &loop, &QEventLoop::quit);
  //  loop.exec();

  int waitingSecs = 60;

  for (;;) {

    if (!m_byteStream) {
      sleep(1);

      if (waitingSecs-- <= 0) {
        // 超时
        qWarning() << "Timout to wait stream open.";
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
      qWarning() << "send error";
      emit fileError(m_friendId, *m_file, m_sentBytes);
      break;
    }
    m_seq += 1;
    m_sentBytes += buf.size();
    emit fileSending(m_friendId, *m_file, m_ack_seq, m_ack_seq * m_buf, false);
  }

  if (m_byteStream) {
    m_byteStream->close();
    emit fileSending(m_friendId, *m_file, m_ack_seq, m_sentBytes, true);
  } else {
    qWarning() << "";
  }

  qFile->close();
  qDebug("finished.");
}

void IMFileTask::abort() { emit fileAbort(m_friendId, *m_file, m_sentBytes); }

void IMFileTask::handleBytestreamOpen(gloox::Bytestream *bs) {
  qDebug() << __func__ << ("file") << qFile->fileName();
  if (!qFile->open(QIODevice::ReadOnly)) {
    return;
  }
  m_byteStream = bs;
}

void IMFileTask::handleBytestreamClose(gloox::Bytestream *bs) {
  qDebug() << __func__ << "closed:" << (qstring(bs->sid()));
  emit fileSent(m_friendId, *m_file);
}

void IMFileTask::handleBytestreamData(gloox::Bytestream *bs, const std::string &data) { qDebug() << __func__ << "data:" << qstring(bs->sid()); }

void IMFileTask::handleBytestreamDataAck(gloox::Bytestream *bs) {
  qDebug() << __func__ << "acked:" << qstring(bs->sid());
  m_ack_seq += 1;
  // 考虑性能关系暂时不处理实时反馈ack
  //   if(m_ack_seq < m_seq){
  //     emit fileSending(m_friendId, m_file, m_ack_seq, m_ack_seq*m_buf,
  //     false);
  //   } else{
  //     emit fileSending(m_friendId, m_file, m_ack_seq, m_sentBytes, true);
  //   }
}

void IMFileTask::handleBytestreamError(gloox::Bytestream *bs, const gloox::IQ &iq) {
  qDebug() << __func__ << qstring(bs->sid());
  fileError(m_friendId, *m_file, m_sentBytes);
}

bool IMFileTask::sendFinished() const { return m_file->size == m_sentBytes; }
bool IMFileTask::ackFinished() const { return m_seq > 0 && m_seq == m_ack_seq; }

void IMFileTask::forceQuit() {
  qDebug() << __func__ << "...";
  quit();
  wait();
  qDebug() << __func__ << "completed.";
}

} // namespace lib::messenger