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

#include "src/audio/iaudiocontrol.h"
#include "src/audio/iaudiosink.h"
#include "src/audio/iaudiosource.h"
#include "src/core/FriendId.h"

class QTimer;
class AudioFilterer;
class CoreVideoSource;
class CoreAV;
class Group;

/**\brief Image Descriptor */
typedef struct vpx_image {
    /* Image storage dimensions */
    unsigned int w;         /**< Stored image width */
    unsigned int h;         /**< Stored image height */
    unsigned int bit_depth; /**< Stored image bit-depth */

    /* Image display dimensions */
    unsigned int d_w; /**< Displayed image width */
    unsigned int d_h; /**< Displayed image height */

    unsigned char* planes[4]; /**< pointer to the top left pixel for each plane */
    int stride[4];            /**< stride between rows for each plane */
} vpx_image_t;                /**< alias for struct vpx_image */

class ToxCall : public QObject {
    Q_OBJECT

protected:
    ToxCall() = delete;
    ToxCall(bool VideoEnabled, CoreAV& av, IAudioControl& audio);
    ~ToxCall();

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

protected:
    bool active{false};
    CoreAV* av{nullptr};
    // audio
    IAudioControl& audio;
    bool muteMic{false};
    bool muteVol{false};
    // video
    CoreVideoSource* videoSource{nullptr};
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
    ToxFriendCall(QString peerId, bool VideoEnabled, CoreAV& av, IAudioControl& audio);
    ToxFriendCall(ToxFriendCall&& other) = delete;

    ~ToxFriendCall();

    ToxFriendCall& operator=(ToxFriendCall&& other) = delete;

    lib::messenger::CallState getState() const;

    void setState(const lib::messenger::CallState& value);

    void playAudioBuffer(const int16_t* data, int samples, unsigned channels, int sampleRate) const;

    const QString& getPeerId() { return peerId; }

private slots:
    void onAudioSourceInvalidated();
    void onAudioSinkInvalidated();

private:
    QMetaObject::Connection audioSinkInvalid;
    lib::messenger::CallState state{lib::messenger::CallState::NONE};
    std::unique_ptr<IAudioSink> sink = nullptr;
    QString peerId;
};

class ToxGroupCall : public ToxCall {
    Q_OBJECT
public:
    ToxGroupCall() = delete;
    ToxGroupCall(const Group& group, CoreAV& av, IAudioControl& audio);
    ToxGroupCall(ToxGroupCall&& other) = delete;
    ~ToxGroupCall();

    ToxGroupCall& operator=(ToxGroupCall&& other) = delete;
    void removePeer(FriendId peerId);

    void playAudioBuffer(const FriendId& peer, const int16_t* data, int samples, unsigned channels,
                         int sampleRate);

private:
    void addPeer(FriendId peerId);
    bool havePeer(FriendId peerId);
    void clearPeers();

    std::map<FriendId, std::unique_ptr<IAudioSink>> peers;
    std::map<FriendId, QMetaObject::Connection> sinkInvalid;
    const Group& group;

private slots:
    void onAudioSourceInvalidated();
    void onAudioSinkInvalidated(FriendId peerId);
};

#endif  // TOXCALL_H
