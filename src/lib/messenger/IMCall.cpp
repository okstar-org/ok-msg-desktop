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
#include "IMJingle.h"
#include "lib/session/AuthSession.h"

namespace lib::messenger {

IMCall::IMCall( QObject *parent): QObject(parent)
{
    session = ok::session::AuthSession::Instance();

    jingle = new IMJingle(session->im(),{}, this);
    connectJingle(jingle);



    std::list<ortc::IceServer> iceServers;

      std::list<ExtDisco::Service> discos;

      ExtDisco::Service disco0;
      disco0.type="turn";
      disco0.host = "chuanshaninfo.com";
      disco0.port=34780;
      disco0.username="gaojie";
      disco0.password="hncs";
      discos.push_back(disco0);

      ExtDisco::Service disco1;
      disco1.type="stun";
      disco1.host = "stun.l.google.com";
      disco1.port=19302;

      discos.push_back(disco1);


      for (const auto &item :  discos) {
        ortc::IceServer ice;
        ice.uri = item.type + ":" + item.host + ":" + std::to_string(item.port);
        //              "?transport=" + item.transport;
        ice.username = item.username;
        ice.password = item.password;
        qDebug() <<"Add ice:" << ice.uri.c_str();
        iceServers.push_back(ice);
      }

      rtcManager  = lib::ortc::OkRTCManager::getInstance(iceServers, nullptr, nullptr);
      auto rtc    = rtcManager->createInstance();
      qDebug()    << "RTC is:" << rtc;

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
  connect(_jingle, &IMJingle
          ::receiveCallRequest, this,
          [&](QString friendId, QString callId, bool audio, bool video) {
            for (auto handler : callHandlers) {
              handler->onCall(friendId, callId, audio, video);
            }
          });

  connect(_jingle, &IMJingle::receiveCallRetract, this,
          [&](QString friendId, int state) {
            for (auto handler : callHandlers) {
              handler->onCallRetract(friendId, state);
            }
          });

  connect(_jingle, &IMJingle::receiveCallAcceptByOther, this,
          [&](const QString& callId, const IMPeerId & peerId) {
            for (auto handler : callHandlers) {
              handler->onCallAcceptByOther(callId, peerId);
            }
          });

  connect(_jingle, &IMJingle::receiveCallStateAccepted, this,
          [&](IMPeerId friendId, QString callId, bool video) {
            for (auto handler : callHandlers) {
              handler->receiveCallStateAccepted(friendId, callId, video);
            }
          });

  connect(_jingle, &IMJingle::receiveCallStateRejected, this,
          [&](IMPeerId friendId, QString callId, bool video) {
            for (auto handler : callHandlers) {
              handler->receiveCallStateRejected(friendId, callId, video);
            }
          });

  connect(_jingle, &IMJingle::receiveFriendHangup, this,
          [&](QString friendId, int state) {
            for (auto handler : callHandlers) {
              handler->onHangup(
                  friendId, (TOXAV_FRIEND_CALL_STATE)state);
            }
          });

  connect(_jingle, &IMJingle::receiveFriendHangup, this,
          [&](QString friendId, int state) {
            for (auto handler : callHandlers) {
              handler->onHangup(friendId, (TOXAV_FRIEND_CALL_STATE)state);
            }
          });

  connect(_jingle, &IMJingle::receiveFriendVideoFrame,
          this,
          [&](const QString &friendId, //
              uint16_t w, uint16_t h,  //
              const uint8_t *y, const uint8_t *u, const uint8_t *v,
              int32_t ystride, int32_t ustride, int32_t vstride) {
            emit receiveFriendVideoFrame(friendId, //
                                         w, h,     //
                                         y, u, v,  //
                                         ystride, ustride, vstride);
          });

  connect(_jingle, &IMJingle::receiveSelfVideoFrame, this,
          [&](uint16_t w, uint16_t h, //
              const uint8_t *y, const uint8_t *u, const uint8_t *v,
              int32_t ystride, int32_t ustride, int32_t vstride) {
            emit receiveSelfVideoFrame(w, h,    //
                                       y, u, v, //
                                       ystride, ustride, vstride);
          });



}



bool IMCall::callToFriend(const QString &friendId, const QString &sId, bool video) {
  qDebug() << __func__ <<"friend:" << friendId <<"sId" << sId << "video?" << video;


  auto im = ok::session::AuthSession::Instance()->im();
  im->proposeJingleMessage(friendId, sId, video);

  auto resources = im->getOnlineResources(stdstring(friendId));
  if (resources.empty()) {
    qWarning() << "Can not find online friends" << friendId;
    return false;
  }

  jingle->proposeJingleMessage(friendId, sId, video);
  qDebug() << "Sent propose jingle message.";


  return true;


//  return _jingle->startCall(f, sId, video);
}

bool IMCall::callToPeerId(const IMPeerId &to,
                                   const QString &sId, bool video) {

  qDebug() << QString("peerId:%1 video:%2").arg((to.toString())).arg(video);
//  return _jingle->createCall(to, sId, video);
  return false;
}

bool IMCall::callAnswerToFriend(const QString &f, const QString &callId,
                               bool video) {
  qDebug() << QString("friend:%1 video:%2").arg((f)).arg(video);
//  return _jingle->answer(f, callId, video);
  return false;
}

bool IMCall::callCancelToFriend(const QString &f, const QString &sId) {
//  _jingle->cancelCall(f, callId);
  return true;
}

void IMCall::setMute(bool mute) {
//    _jingle->setMute(mute);
}

void IMCall::setRemoteMute(bool mute) {
    //    _jingle->setRemoteMute(mute);
}


}
