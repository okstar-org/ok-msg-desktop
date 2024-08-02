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
// Created by gaojie on 24-5-29.
//
#include <iostream>

#include "IMCall.h"
#include "lib/ortc/ok_rtc_defs.h"
#include "lib/session/AuthSession.h"
#include "lib/ortc/ok_rtc.h"
#include "lib/ortc/ok_rtc_defs.h"
#include "lib/ortc/ok_rtc_manager.h"

namespace lib::messenger {

IMCall::IMCall(IM* im, QObject *parent): IMJingle(im, parent)
{
    jingle = IMCall::getInstance();
    connectJingle(jingle);
}

void IMCall::addCallHandler(CallHandler *hdr)
{
    callHandlers.push_back(hdr);
}

bool IMCall::callToGroup(const QString &g)
{
    return false;
}

void IMCall::connectJingle(IMJingle* _jingle) {
  qDebug()<<__func__<<_jingle;

  /**
   * callHandlers
   */
  connect(_jingle, &IMCall
          ::receiveCallRequest, this,
          [&](IMPeerId peerId, QString callId, bool audio, bool video) {
            for (auto handler : callHandlers) {
              handler->onCall(peerId, callId, audio, video);
            }
          });

  connect(_jingle, &IMCall::receiveCallRetract, this,
          [&](QString friendId, int state) {
            for (auto handler : callHandlers) {
              handler->onCallRetract(friendId, state);
            }
          });

  connect(_jingle, &IMCall::receiveCallAcceptByOther, this,
          [&](const QString& callId, const IMPeerId & peerId) {
            for (auto handler : callHandlers) {
              handler->onCallAcceptByOther(callId, peerId);
            }
          });

  connect(_jingle, &IMCall::receiveCallStateAccepted, this, &IMCall::onCallAccepted);

  connect(_jingle, &IMCall::receiveCallStateRejected, this,
          [&](IMPeerId friendId, QString callId, bool video) {
            for (auto handler : callHandlers) {
              handler->receiveCallStateRejected(friendId, callId, video);
            }
          });

  connect(_jingle, &IMCall::receiveFriendHangup, this,
          [&](QString friendId, int state) {
            for (auto handler : callHandlers) {
              handler->onHangup(
                  friendId, (TOXAV_FRIEND_CALL_STATE)state);
            }
          });

  connect(_jingle, &IMCall::receiveFriendHangup, this,
          [&](QString friendId, int state) {
            for (auto handler : callHandlers) {
              handler->onHangup(friendId, (TOXAV_FRIEND_CALL_STATE)state);
            }
          });

//  connect(_jingle, &IMCall::receiveFriendVideoFrame,
//          this,
//          [&](const QString &friendId, //
//              uint16_t w, uint16_t h,  //
//              const uint8_t *y, const uint8_t *u, const uint8_t *v,
//              int32_t ystride, int32_t ustride, int32_t vstride) {
//            emit receiveFriendVideoFrame(friendId, //
//                                         w, h,     //
//                                         y, u, v,  //
//                                         ystride, ustride, vstride);
//          });
//
//  connect(_jingle, &IMCall::receiveSelfVideoFrame, this,
//          [&](uint16_t w, uint16_t h, //
//              const uint8_t *y, const uint8_t *u, const uint8_t *v,
//              int32_t ystride, int32_t ustride, int32_t vstride) {
//            emit receiveSelfVideoFrame(w, h,    //
//                                       y, u, v, //
//                                       ystride, ustride, vstride);
//          });
}

//对方接受呼叫
void IMCall::onCallAccepted(IMPeerId peerId, QString callId, bool video)
{
    qDebug() << __func__
             << "peerId:" << peerId.toString()
             << "sId" << callId
             << "video?" << video;
    for (auto handler : callHandlers) {
        handler->receiveCallStateAccepted(peerId, callId, video);
    }
}



bool IMCall::callToFriend(const QString &friendId, const QString &sId, bool video) {
  qDebug() << __func__ <<"friend:" << friendId <<"sId" << sId << "video?" << video;

  jingle->proposeJingleMessage(friendId, sId, video);

  auto resources = im->getOnlineResources(stdstring(friendId));
  if (resources.empty()) {
    qWarning() << "Can not find online friends" << friendId;
    return false;
  }

  jingle->proposeJingleMessage(friendId, sId, video);
  qDebug() << "Sent propose jingle message.";
  return true;
}

bool IMCall::callToPeerId(const IMPeerId &to, const QString &sId, bool video) {
  qDebug() << QString("peerId:%1 video:%2").arg((to.toString())).arg(video);
  auto r = createCall(to, sId, video );
  qDebug() <<"createdCall=>" << r;
  return r;
}

bool IMCall::callAnswerToFriend(const IMPeerId &f, const QString &callId, bool video) {
  return answer(f, callId, video);
}

void IMCall::callRetract(const IMContactId &f, const QString &sId) {
    cancelCall(f, sId);
}

void IMCall::callReject(const IMPeerId &f, const QString &sId)
{
    rejectCall(f, sId);
}


// startCall
bool IMCall::startCall(const QString &friendId, const QString &sId, bool video) {
  qDebug()<<__func__<<"friendId:"<<friendId<<"video:"<<video;
  
  auto resources = im->getOnlineResources(stdstring(friendId));
  if (resources.empty()) {
    qWarning() << "目标用户不在线！";
    return false;
  }

  sendCallToResource(friendId, sId, video);
  return true;
}

bool IMCall::sendCallToResource(const QString &friendId, const QString &sId,bool video) {
  proposeJingleMessage(friendId, sId, video);
  return true;
}


bool IMCall::createCall(const IMPeerId &to, const QString &sId, bool video) {
  qDebug()<<__func__<< "to:" << to.toString() << "sId:" << sId;

  auto ws = createSession(to, sId, lib::ortc::JingleCallType::av);

  auto rtc = lib::ortc::OkRTCManager::getInstance()->getRtc();
  rtc->addRTCHandler(this);

  bool createdCall = rtc->call(stdstring(to.toString()), stdstring(sId), video);
  if(createdCall){
    ws->createOffer(stdstring(to.toString()));
  }

  return createdCall;
}

void IMCall::cancel(const QString &friendId) {
  qDebug()<<__func__<<friendId;

  auto sId = m_friendSessionMap.value(IMPeerId(friendId));

  auto session = m_sessionMap.value(sId);
  if (session) {
    cancelCall(IMContactId{friendId}, qstring(session->getSession()->sid()));
    clearSessionInfo(session->getSession());
  }

}

void IMCall::cancelCall(const IMContactId &friendId, const QString &sId) {
  qDebug()<< __func__ << friendId.toString() << sId;

  IMJingleSession *s = findSession(sId);
  if (s) {
    s->doTerminate();
    s->setCallStage(CallStage::StageNone);
    clearSessionInfo(s->getSession());
  }
  retractJingleMessage(friendId.toString(), sId);
  //  else {
  // jingle-message
  //    if (s->direction() == CallDirection:: CallOut) {
  //    } else if (s->direction() == CallDirection:: CallIn) {
  //      rejectJingleMessage(friendId.toString(), sId);
  //    }
  //  }
}

void IMCall::rejectCall(const IMPeerId &peerId, const QString &sId)
{
  qDebug()<< __func__ << peerId.toString() << sId;

  IMJingleSession *s = findSession(sId);
  if (s) {
    s->doTerminate();
    clearSessionInfo(s->getSession());
  }else
  {
    rejectJingleMessage(peerId.toString(), sId);
  }
}

bool IMCall::answer(const IMPeerId &peerId, const QString &callId, bool video) {

  qDebug()<<__func__<< "peer:" << peerId.toString() << "video:"<<  video ;

  auto rtc = lib::ortc::OkRTCManager::getInstance()->getRtc();
  rtc->addRTCHandler(this);

  acceptJingleMessage(peerId, callId, video);

  return true;
}


void IMCall::setMute(bool mute) {
  lib::ortc::OkRTCManager::getInstance()->getRtc()->setMute(mute);
}

void IMCall::setRemoteMute(bool mute) {
  lib::ortc::OkRTCManager::getInstance()->getRtc()->setRemoteMute(mute);
}


void IMCall::onCreatePeerConnection(const std::string &sId,
                                      const std::string &peerId,
                                      bool ok) {
  auto p = qstring(peerId);
  auto s = qstring(sId);
  
  qDebug()<<__func__<< "sId:" << s
           << "peerId:" << p
           << "isOk=>" << ok;

  //  emit call->sig_createPeerConnection(s, p, ok);
}

void IMCall::onRTP(const std::string &sid,    //
                     const std::string &peerId, //
                     const lib::ortc::OJingleContentAv &oContext) {
  auto sId = qstring(sid);
  qDebug()<<__func__ << "sId:"<<sId<<"peerId:"<<qstring(peerId);

  PluginList plugins;
  oContext.toPlugins(plugins);

  auto pSession = findSession(sId);
  if (!pSession) {
    qWarning() << "Unable to find session" << &sId;
    return;
  }

  if (pSession->direction() == CallDirection::CallIn) {
    pSession->getSession()->sessionAccept(plugins);
  } else if (pSession->direction() == CallDirection::CallOut) {
    pSession->getSession()->sessionInitiate(plugins);
  }
}

void IMCall::onIce(const std::string &sId,    //
                     const std::string &peerId, //
                     const lib::ortc::OIceUdp &oIceUdp) {

  auto sid = qstring(sId);

  qDebug()<< __func__
           << "sId:"<< sid
           << "peerId:" << qstring(peerId)
           << "mid:" << qstring(oIceUdp.mid)
           << "mline:" << oIceUdp.mline;

  auto *session = findSession(sid);
  if (!session) {
    qWarning() << "Unable to find session:" << &sId;
    return;
  }


  auto *iceUdp = new ICEUDP(oIceUdp.pwd, oIceUdp.ufrag, oIceUdp.candidates);
  iceUdp->setDtls(oIceUdp.dtls);

  PluginList pluginList;
  pluginList.emplace_back(iceUdp);
  auto c = new Jingle::Content(oIceUdp.mid, pluginList);
  session->getSession()->transportInfo(c);
}

/**
 * 视频渲染
 * @param peerId
 * @param image
 */
void IMCall::onRender(const std::string &peerId, lib::ortc::RendererImage image) {
  if (peerId.empty()) {
    emit receiveSelfVideoFrame(image.width_, image.height_, image.y, image.u,
                               image.v, image.ystride, image.ustride,
                               image.vstride);
  } else {
    emit receiveFriendVideoFrame(IMPeerId(peerId).toFriendId(), image.width_,
                                 image.height_, image.y, image.u, image.v,
                                 image.ystride, image.ustride, image.vstride);
  }
}



}
