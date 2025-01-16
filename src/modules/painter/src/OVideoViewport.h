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

#include "playerwidget.h"

#include <QWidget>
#include <mutex>

#include "lib/network/NetworkHttp.h"

namespace Ui {
class OVideoViewport;
}

namespace module::painter {

class OVideoViewport : public QWidget {
    Q_OBJECT
public:
    explicit OVideoViewport(QWidget* parent = nullptr);
    virtual ~OVideoViewport() override;

    const std::list<PlayerWidget*> playerWidgets() const;

    PlayerWidget* findByUserJID(const ok::base::Jid& jid);

    bool isExist(const ok::base::Jid& jid);

    const std::list<ok::base::Jid> isCheckedUsers() const;

    PlayerWidget* findMine();

protected:
    virtual void showEvent(QShowEvent* e) override;

private:
    Ui::OVideoViewport* ui;

    QHBoxLayout* _hLayout;
    QGridLayout* _gridLayout;

    UserJIDMap _students;

    std::unique_ptr<QTimer> _timer;

    base::DelayedCallTimer* _delayCaller;

    std::mutex _JoinLeftMutex;

    int _showTimes = 0;

    void getRowColForStudent(int& ir, int& ic);

signals:
    void recvUserInfo(const UserInfo& info);
    void voiceMute(bool mute);

public slots:
    void timerUp();
    void onJoined(const UserJID& jid);
    void onLeft(const UserJID& jid);

    void onPeerStatus(const UserJID& jid, const PeerStatus& status);

    void onRecvUserInfo(const UserInfo& userInfo);
};

}  // namespace module::painter
