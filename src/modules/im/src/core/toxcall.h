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

#ifndef TOXCALL_H
#define TOXCALL_H

#include "src/audio/iaudiocontrol.h"
#include "src/audio/iaudiosink.h"
#include "src/audio/iaudiosource.h"
#include <src/core/toxpk.h>
#include "lib/messenger/tox/tox.h"

#include <QMap>
#include <QMetaObject>
#include <QtGlobal>

#include <cstdint>
#include <memory>

class QTimer;
class AudioFilterer;
class CoreVideoSource;
class CoreAV;
class Group;

class ToxCall : public QObject {
  Q_OBJECT

protected:
  ToxCall() = delete;
  ToxCall(bool VideoEnabled, CoreAV &av, IAudioControl &audio);
  ~ToxCall();

public:
  ToxCall(const ToxCall &other) = delete;
  ToxCall(ToxCall &&other) = delete;

  ToxCall &operator=(const ToxCall &other) = delete;
  ToxCall &operator=(ToxCall &&other) = delete;

  bool isActive() const;
  void setActive(bool value);

  bool getMuteVol() const;
  void setMuteVol(bool value);

  bool getMuteMic() const;
  void setMuteMic(bool value);

  bool getVideoEnabled() const;
  void setVideoEnabled(bool value);

  bool getNullVideoBitrate() const;
  void setNullVideoBitrate(bool value);

  CoreVideoSource *getVideoSource() const;

  QString getCallId() const;
  void setCallId(QString value);
protected:
  bool active{false};
  CoreAV *av{nullptr};
  // audio
  IAudioControl &audio;
  bool muteMic{false};
  bool muteVol{false};
  // video
  CoreVideoSource *videoSource{nullptr};
  QMetaObject::Connection videoInConn;
  bool videoEnabled{false};
  bool nullVideoBitrate{false};
  std::unique_ptr<IAudioSource> audioSource = nullptr;
  QString callId;
};

class ToxFriendCall : public ToxCall {
  Q_OBJECT
public:
  ToxFriendCall() = delete;
  ToxFriendCall(QString friendId, bool VideoEnabled, CoreAV &av,
                IAudioControl &audio);
  ToxFriendCall(ToxFriendCall &&other) = delete;
  ToxFriendCall &operator=(ToxFriendCall &&other) = delete;
  ~ToxFriendCall();

  TOXAV_FRIEND_CALL_STATE getState() const;
  void setState(const TOXAV_FRIEND_CALL_STATE &value);

  void playAudioBuffer(const int16_t *data, int samples, unsigned channels,
                       int sampleRate) const;

private slots:
  void onAudioSourceInvalidated();
  void onAudioSinkInvalidated();

private:
  QMetaObject::Connection audioSinkInvalid;
  TOXAV_FRIEND_CALL_STATE state{TOXAV_FRIEND_CALL_STATE_NONE};
  std::unique_ptr<IAudioSink> sink = nullptr;
  QString friendId;
};

class ToxGroupCall : public ToxCall {
  Q_OBJECT
public:
  ToxGroupCall() = delete;
  ToxGroupCall(const Group &group, CoreAV &av, IAudioControl &audio);
  ToxGroupCall(ToxGroupCall &&other) = delete;
  ~ToxGroupCall();

  ToxGroupCall &operator=(ToxGroupCall &&other) = delete;
  void removePeer(ToxPk peerId);

  void playAudioBuffer(const ToxPk &peer, const int16_t *data, int samples,
                       unsigned channels, int sampleRate);

private:
  void addPeer(ToxPk peerId);
  bool havePeer(ToxPk peerId);
  void clearPeers();

  std::map<ToxPk, std::unique_ptr<IAudioSink>> peers;
  std::map<ToxPk, QMetaObject::Connection> sinkInvalid;
  const Group &group;

private slots:
  void onAudioSourceInvalidated();
  void onAudioSinkInvalidated(ToxPk peerId);
};

#endif // TOXCALL_H
