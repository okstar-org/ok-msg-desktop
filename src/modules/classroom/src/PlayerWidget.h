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

#include <QFrame>
#include <QGridLayout>
#include <QObject>
#include <QWidget>

#include "User.h"
#include "base/jid.h"
#include "component/OVideoWidget.h"
#include "component/VideoWidget.h"
#include "ui.h"

namespace module::classroom {

class PlayerWidget;

class PlayerWidgetShadow : public QFrame {
    Q_OBJECT
public:
    PlayerWidgetShadow(PlayerWidget* player, QWidget* parent = nullptr);

    PlayerWidget* player() const {
        return _player;
    }

private:
    PlayerWidget* _player;
};

class PlayerWidget : public QWidget {
    Q_OBJECT
public:
    explicit PlayerWidget(VideoWidgetConfig config, QWidget* parent = nullptr);

    ~PlayerWidget() override;

    void setViewSize(VIDEO_SIZE size);

    void start();

    void stop();

    void setBorderRed();
    void setBorderBlue();

    void setChecked(bool checked);
    void setMute(bool checked);

    virtual const ok::base::Jid& peerJID() {
        return _jid;
    }
    virtual void setPeerJID(const ok::base::Jid& jid) {
        _jid = jid;
    }

    inline virtual const PlayerWidgetShadow* shadow() {
        return _shadow;
    }

    virtual void setPeerStatus(const PeerStatus& status);

    virtual void setPeerName(const QString& name);

    virtual void setDisabled();

    virtual bool left() {
        return _left;
    }

    virtual bool disabled() {
        return false;  // return _videoWidget->disabled();
    }

    virtual bool isChecked() const {
        return _checked;
    }

protected:
    virtual void showEvent(QShowEvent* e) override;
    virtual void mousePressEvent(QMouseEvent* event) override;
    virtual void mouseReleaseEvent(QMouseEvent* event) override;
    virtual void mouseDoubleClickEvent(QMouseEvent* event) override;
    virtual void resizeEvent(QResizeEvent* event) override;

private:
    VideoWidgetConfig _config;
    ok::base::Jid _jid;

    bool _checked = false;

    bool _mute = false;

    bool _left = false;

    int _showTimes = 0;

    QWidget* _parent = nullptr;

    QLayout* _frame = nullptr;

    QGridLayout* _gLayout = nullptr;

    OVideoWidget* _videoWidget;

    VideoOverlay* _overlay;

    PlayerWidgetShadow* _shadow;

signals:

public slots:

    void goBack();

    void addSilverCoin(int coin);

    //    void onCtrollerChecked(WB_CTRL ctrl, bool isChecked) ;
    //    void onPeerStatus(PeerStatus status);
};

}  // namespace module::classroom
