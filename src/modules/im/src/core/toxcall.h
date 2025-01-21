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

#include <cstdint>
#include <memory>

#include <QMap>
#include <QMetaObject>
#include <QtGlobal>

#include "lib/audio/iaudiocontrol.h"
#include "lib/audio/iaudiosink.h"
#include "lib/audio/iaudiosource.h"
#include "src/model/FriendId.h"

class QTimer;
namespace module::im {

class AudioFilterer;
class CoreVideoSource;
class CoreAV;
class Group;

class ToxCall : public QObject {
    Q_OBJECT

protected:
    ToxCall() = delete;
    explicit ToxCall(lib::messenger::CallDirection direction,
                     CoreAV& av,
                     lib::audio::IAudioControl& audio,
                     bool videoEnabled = false);
    ~ToxCall() override;

public:
    ToxCall(const ToxCall& other) = delete;
    ToxCall(ToxCall&& other) = delete;

    ToxCall& operator=(const ToxCall& other) = delete;
    ToxCall& operator=(ToxCall&& other) = delete;

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

    CoreVideoSource* getVideoSource() const;

    QString getCallId() const;
    void setCallId(QString value);

    inline const lib::ortc::CtrlState& getCtrlState() const {
        return ctrlState;
    }

    [[nodiscard]] inline lib::messenger::CallState getState() const {
        return state;
    };

    [[nodiscard]] inline lib::messenger::CallDirection getDirection() const {
        return direction;
    };

protected:
    bool active = false;
    CoreAV* av = nullptr;
    // audio
    lib::audio::IAudioControl& audio;

    // video
    CoreVideoSource* videoSource = nullptr;
    QMetaObject::Connection videoInConn;

    bool nullVideoBitrate = false;

    std::unique_ptr<lib::audio::IAudioSource> audioSource;

    // 呼叫ID
    QString callId;

    // 控制状态
    lib::ortc::CtrlState ctrlState;
    // 呼叫方向
    lib::messenger::CallDirection direction;
    // 呼叫状态
    lib::messenger::CallState state;
    // 呼叫状态机
    lib::messenger::CallFSM callFsm;
};

class ToxFriendCall : public ToxCall {
    Q_OBJECT
public:
    ToxFriendCall() = delete;
    ToxFriendCall(QString peerId,
                  lib::messenger::CallDirection direction,
                  CoreAV& av,
                  lib::audio::IAudioControl& audio,
                  bool videoEnabled);
    ToxFriendCall(ToxFriendCall&& other) = delete;

    ~ToxFriendCall() override;

    ToxFriendCall& operator=(ToxFriendCall&& other) = delete;

    void setState(const lib::messenger::CallState& value);

    void playAudioBuffer(const int16_t* data, int samples, unsigned channels, int sampleRate) const;

    const QString& getPeerId() {
        return peerId;
    }

private slots:
    void onAudioSourceInvalidated();
    void onAudioSinkInvalidated();

private:
    QMetaObject::Connection audioSinkInvalid;
    std::unique_ptr<lib::audio::IAudioSink> sink = nullptr;
    QString peerId;
};

class ToxGroupCall : public ToxCall {
    Q_OBJECT
public:
    ToxGroupCall() = delete;
    ToxGroupCall(const Group& group,
                 lib::messenger::CallDirection direction,
                 CoreAV& av,
                 lib::audio::IAudioControl& audio,
                 bool videoEnabled);
    ToxGroupCall(ToxGroupCall&& other) = delete;
    ~ToxGroupCall() override;

    ToxGroupCall& operator=(ToxGroupCall&& other) = delete;
    void removePeer(FriendId peerId);

    void playAudioBuffer(const FriendId& peer, const int16_t* data, int samples, unsigned channels,
                         int sampleRate);

private:
    void addPeer(FriendId peerId);
    bool havePeer(FriendId peerId);
    void clearPeers();

    std::map<FriendId, std::unique_ptr<lib::audio::IAudioSink>> peers;
    std::map<FriendId, QMetaObject::Connection> sinkInvalid;
    const Group& group;

private slots:
    void onAudioSourceInvalidated();
    void onAudioSinkInvalidated(FriendId peerId);
};
}  // namespace module::im
#endif  // TOXCALL_H
