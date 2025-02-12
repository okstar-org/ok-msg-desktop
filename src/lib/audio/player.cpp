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


#include "player.h"

#include <QMediaPlayer>
#include <QMediaPlaylist>


namespace lib::audio{

QString PlayStateAsStr(PlayState s)
{
    static std::vector<std::string> ls{"Stopped", "Playing", "Paused"};
    return QString::fromStdString(ls.at((int)s));
}

Player::Player(QObject *parent) : QObject(parent)
{
    m_playlist = new QMediaPlaylist(this);
    m_player = new QMediaPlayer(this);
    m_player->setPlaylist(m_playlist);

    connect(m_player, &QMediaPlayer::stateChanged, this,
            [this](QMediaPlayer::State newState){
                emit stateChanged(file, (PlayState)newState);
                if(newState == QMediaPlayer::StoppedState){
                    file.clear();
                }
            });
}

Player::~Player() = default;

void Player::play(const QString &filePath)
{
    file = filePath;
    m_playlist->clear();
    m_playlist->addMedia(QUrl::fromLocalFile(filePath));
    m_player->play();
}

void Player::pause()
{
    m_player->pause();
}

void Player::stop()
{
    m_player->stop();
}



}
