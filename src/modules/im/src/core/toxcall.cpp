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

#include "src/core/toxcall.h"
#include <QTimer>
#include "lib/audio/audio.h"
#include "src/core/coreav.h"
#include "src/model/group.h"
#include "src/video/corevideosource.h"
namespace module::im {

ToxCall::ToxCall(bool VideoEnabled, CoreAV& av, lib::audio::IAudioControl& audio)
        : av{&av}, audio(audio), ctrlState({true, VideoEnabled, true}) {
    audioSource = audio.makeSource();
}

ToxCall::~ToxCall() {
    qDebug() << __func__;
    if (ctrlState.enableCam) {
        QObject::disconnect(videoInConn);
        //    CameraSource::getInstance().unsubscribe();
    }
}

bool ToxCall::isActive() const {
    return active;
}

void ToxCall::setActive(bool value) {
    active = value;
}

bool ToxCall::getMuteVol() const {
    return !ctrlState.enableSpk;
}

void ToxCall::setMuteVol(bool value) {
    ctrlState.enableSpk = !value;
}

bool ToxCall::getMuteMic() const {
    return !ctrlState.enableMic;
}

void ToxCall::setMuteMic(bool value) {
    ctrlState.enableMic = !value;
}

bool ToxCall::getVideoEnabled() const {
    return ctrlState.enableCam;
}

void ToxCall::setVideoEnabled(bool value) {
    ctrlState.enableCam = value;
}

bool ToxCall::getNullVideoBitrate() const {
    return nullVideoBitrate;
}

void ToxCall::setNullVideoBitrate(bool value) {
    nullVideoBitrate = value;
}

CoreVideoSource* ToxCall::getVideoSource() const {
    return videoSource;
}

QString ToxCall::getCallId() const {
    return callId;
}

void ToxCall::setCallId(QString value) {
    callId = value;
}

ToxFriendCall::ToxFriendCall(QString peerId, bool VideoEnabled, CoreAV& av,
                             lib::audio::IAudioControl& audio)
        : ToxCall(VideoEnabled, av, audio)
        , sink(audio.makeSink())
        , peerId{peerId}

{
    connect(audioSource.get(), &lib::audio::IAudioSource::frameAvailable, this,
            [this](const int16_t* pcm, size_t samples, uint8_t chans, uint32_t rate) {
                this->av->sendCallAudio(this->peerId, pcm, samples, chans, rate);
            });

    connect(audioSource.get(), &lib::audio::IAudioSource::invalidated, this,
            &ToxFriendCall::onAudioSourceInvalidated);

    if (sink) {
        audioSinkInvalid =
                sink->connectTo_invalidated(this, [this]() { this->onAudioSinkInvalidated(); });
    }

    // register video
    if (ctrlState.enableCam) {
        videoSource = new CoreVideoSource();
        // CameraSource& source = CameraSource::getInstance();

        // if (source.isNone()) {
        //     source.setupDefault();
        // }
        // source.subscribe();
        // videoInConn = QObject::connect(&source, &VideoSource::frameAvailable,
        //                                [&av,
        //                                receiver](std::shared_ptr<VideoFrame>
        //                                frame) {
        //                                    av.sendCallVideo(receiver, frame);
        //                                });
        // if (!videoInConn) {
        //     qDebug() << "Video connection not working";
        // }
    }
}

ToxFriendCall::~ToxFriendCall() {
    qDebug() << __func__;
}

void ToxFriendCall::onAudioSourceInvalidated() {
    //  auto newSrc = audio.makeSource();
    //  connect(
    //      newSrc.get(), &IAudioSource::frameAvailable, this,
    //      [this](const int16_t *pcm, size_t samples, uint8_t chans, uint32_t rate) {
    //        this->av->sendCallAudio(this->friendId, pcm, samples, chans, rate);
    //      });
    //  audioSource = std::move(newSrc);

    //  connect(audioSource.get(), &IAudioSource::invalidated, this,
    //          &ToxFriendCall::onAudioSourceInvalidated);
}

void ToxFriendCall::onAudioSinkInvalidated() {
    auto newSink = audio.makeSink();

    if (newSink) {
        audioSinkInvalid =
                newSink->connectTo_invalidated(this, [this]() { this->onAudioSinkInvalidated(); });
    }

    sink = std::move(newSink);
}

lib::messenger::CallState ToxFriendCall::getState() const {
    return state;
}

void ToxFriendCall::setState(const lib::messenger::CallState& value) {
    state = value;
}

void ToxFriendCall::playAudioBuffer(const int16_t* data, int samples, unsigned channels,
                                    int sampleRate) const {
    if (sink) {
        sink->playAudioBuffer(data, samples, channels, sampleRate);
    }
}

ToxGroupCall::ToxGroupCall(const Group& group, CoreAV& av, lib::audio::IAudioControl& audio)
        : ToxCall(false, av, audio)
        ,  //
        group{group} {
    // register audio
    connect(audioSource.get(),              //
            &lib::audio::IAudioSource::frameAvailable,  //
            this,                           //
            [this](const int16_t* pcm, size_t samples, uint8_t chans, uint32_t rate) {
                if (this->group.getPeersCount() <= 1) {
                    return;
                }
                this->av->sendGroupCallAudio(this->group.getIdAsString(), pcm, samples, chans,
                                             rate);
            });

    connect(audioSource.get(),           //
            &lib::audio::IAudioSource::invalidated,  //
            this,                        //
            &ToxGroupCall::onAudioSourceInvalidated);
}

ToxGroupCall::~ToxGroupCall() {
    // disconnect all Qt connections
    clearPeers();
}

void ToxGroupCall::onAudioSourceInvalidated() {
    auto newSrc = audio.makeSource();
    connect(audioSource.get(), &lib::audio::IAudioSource::frameAvailable,
            [this](const int16_t* pcm, size_t samples, uint8_t chans, uint32_t rate) {
                if (this->group.getPeersCount() <= 1) {
                    return;
                }

                this->av->sendGroupCallAudio(this->group.getIdAsString(), pcm, samples, chans,
                                             rate);
            });

    audioSource = std::move(newSrc);

    connect(audioSource.get(), &lib::audio::IAudioSource::invalidated, this,
            &ToxGroupCall::onAudioSourceInvalidated);
}

void ToxGroupCall::onAudioSinkInvalidated(FriendId peerId) {
    removePeer(peerId);
    addPeer(peerId);
}

void ToxGroupCall::removePeer(FriendId peerId) {
    const auto& source = peers.find(peerId);
    if (source == peers.cend()) {
        qDebug() << "Peer:" << peerId.toString() << "does not have a source, can't remove";
        return;
    }

    peers.erase(source);
    QObject::disconnect(sinkInvalid[peerId]);
    sinkInvalid.erase(peerId);
}

void ToxGroupCall::addPeer(FriendId peerId) {
    std::unique_ptr<lib::audio::IAudioSink> newSink = audio.makeSink();

    QMetaObject::Connection con;

    if (newSink) {
        con = newSink->connectTo_invalidated(
                this, [this, peerId]() { this->onAudioSinkInvalidated(peerId); });
    }

    peers.emplace(peerId, std::move(newSink));
    sinkInvalid.insert({peerId, con});
}

bool ToxGroupCall::havePeer(FriendId peerId) {
    const auto& source = peers.find(peerId);
    return source != peers.cend();
}

void ToxGroupCall::clearPeers() {
    peers.clear();
    for (auto con : sinkInvalid) {
        QObject::disconnect(con.second);
    }

    sinkInvalid.clear();
}

void ToxGroupCall::playAudioBuffer(const FriendId& peer, const int16_t* data, int samples,
                                   unsigned channels, int sampleRate) {
    if (!havePeer(peer)) {
        addPeer(peer);
    }
    const auto& source = peers.find(peer);
    if (source->second) {
        source->second->playAudioBuffer(data, samples, channels, sampleRate);
    }
}
}  // namespace module::im