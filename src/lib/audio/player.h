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

#include <QObject>


class QMediaPlayer;
class QMediaPlaylist;

namespace lib::audio {


enum class PlayState
{
    Stopped,
    Playing,
    Paused,
};

QString PlayStateAsStr(PlayState s);


class Player : public QObject {
    Q_OBJECT
public:
    explicit Player(QObject* parent = nullptr);
    ~Player() override;

    // 播放指定文件
    void play(const QString &filePath);
    // 暂停播放
    void pause();
    // 停止播放
    void stop();

            // 获取当前播放状态
            // QMediaPlayer::PlaybackState state() const;


private:
    QString file;
    QMediaPlayer* m_player;
    QMediaPlaylist* m_playlist;

signals:
    void stateChanged(QString file, PlayState state);

private slots:
    // void onPlaybackStateChanged(QAudioPlayer::PlaybackState state);

};
}

