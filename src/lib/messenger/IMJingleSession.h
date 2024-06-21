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

#pragma once
#include <QMap>
#include "IM.h"
#include "IMFile.h"
#include "IMFileTask.h"
#include "base/basic_types.h"
#include "lib/ortc/ok_rtc_defs.h"
#include "lib/ortc/ok_rtc_manager.h"
#include <jinglesession.h>


namespace ortc{
struct IceServer;
}

namespace lib {
namespace messenger {

enum CallStage {
  StageNone,
  StageMessage, // XEP-0353: Jingle Message Initiation
  StageSession  // XEP-0166: Jingle https://xmpp.org/extensions/xep-0166.html
};

using namespace gloox;
using namespace gloox::Jingle;

class IMJingleSession : public QObject {
    Q_OBJECT
public:
  explicit IMJingleSession(IM* im,
                           const IMPeerId &peerId,
                           const QString &sId,
                           lib::ortc::JingleCallType callType,
                           Session *mSession,
                           std::vector<FileHandler *>* fileHandlers,
                           ortc::OkRTCHandler *handler);
  virtual ~IMJingleSession();


  [[nodiscard]] Session *getSession() const;
  [[nodiscard]] inline const ortc::OJingleContent &getContext() const {
    return context;
  }

  void onAccept();
  //被动结束
  void onTerminate();
  //主动结束
  void doTerminate();

  void createOffer(const std::string &peerId);

  void setContext(const ortc::OJingleContent &);

  const Session::Jingle *getJingle() const;
  void setJingle(const Session::Jingle *jingle);

  [[nodiscard]] CallDirection direction() const;

  void setCallStage(CallStage state);

  void setAccepted(bool y) { accepted = y; }

  [[nodiscard]] bool isAccepted() const { return accepted; }


  const QString & getId() const {
    return sId;
  }

  void appendIce(const ortc::OIceUdp& ice){
    pendingIceCandidates.emplace_back(ice);
  }

  void pollIce(Fn<void(const ortc::OIceUdp&)> fn){
    while (!pendingIceCandidates.empty()){
      fn(pendingIceCandidates.back());
      pendingIceCandidates.pop_back();
    }
  }

  void addFile(const File &f) {
      m_waitSendFiles.append(f);
  }
  /**
   * 启动文件发送任务
   * @param session
   * @param file
   */
  void doStartFileSendTask(const Jingle::Session *session,
                           const File &file);

  /**
   * 停止文件发送任务
   * @param session
   * @param file
   */
  void doStopFileSendTask(const Jingle::Session *session,
                          const File &file);
private:
  IM* im;
  QString sId;
  Session *session;
  const Session::Jingle *jingle;
  ortc::OJingleContent context;



  lib::ortc::JingleCallType m_callType;
  CallStage m_callStage;
  bool accepted;

  //file
  QList<File> m_waitSendFiles;
  //k: file.id
  QMap<QString, IMFileTask *> m_fileSenderMap;
  std::vector<FileHandler *> *fileHandlers;
  std::list<ortc::OIceUdp> pendingIceCandidates;

signals:
  void sendFileInfo(const QString &friendId, const File &file,
                    int m_seq, int m_sentBytes, bool end);

  void sendFileAbort(const QString &friendId, const File &file,
                     int m_sentBytes);
  void sendFileError(const QString &friendId, const File &file,
                     int m_sentBytes);

};

} // namespace IM
} // namespace lib
