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

#include <QTime>
#include <map>

#include <QtCore/QObject>
#include <QtCore/QThread>

#include "qbasic_types.h"

namespace base {

//    TimeMs getMs( ) {
//        return (TimeMs)QTime::currentTime().msec();
//    }

class Timer final : private QObject {
public:
    explicit Timer(QThread* thread, ok::base::Fn<void()> callback = nullptr);

    explicit Timer(ok::base::Fn<void()> callback = nullptr);

    static Qt::TimerType DefaultType(TimeMs timeout) {
        constexpr auto kThreshold = TimeMs(1000);
        return (timeout > kThreshold) ? Qt::CoarseTimer : Qt::PreciseTimer;
    }

    void setCallback(ok::base::Fn<void()> callback) { _callback = std::move(callback); }

    void callOnce(TimeMs timeout) { callOnce(timeout, DefaultType(timeout)); }

    void callEach(TimeMs timeout) { callEach(timeout, DefaultType(timeout)); }

    void callOnce(TimeMs timeout, Qt::TimerType type) { start(timeout, type, Repeat::SingleShot); }

    void callEach(TimeMs timeout, Qt::TimerType type) { start(timeout, type, Repeat::Interval); }

    bool isActive() const { return (_timerId != 0); }

    void cancel();
    TimeMs remainingTime() const;

    static void Adjust();

protected:
    void timerEvent(QTimerEvent* e) override;

private:
    enum class Repeat : unsigned {
        Interval = 0,
        SingleShot = 1,
    };
    void start(TimeMs timeout, Qt::TimerType type, Repeat repeat);
    void adjust();

    void setTimeout(TimeMs timeout);
    int timeout() const;

    void setRepeat(Repeat repeat) { _repeat = static_cast<unsigned>(repeat); }
    Repeat repeat() const { return static_cast<Repeat>(_repeat); }

    ok::base::Fn<void()> _callback;
    TimeMs _next = 0;
    int _timeout = 0;
    int _timerId = 0;

    Qt::TimerType _type : 2;
    bool _adjusted : 1;
    unsigned _repeat : 1;
};

class DelayedCallTimer final : private QObject {
public:
    int call(TimeMs timeout, ok::base::Fn<void()> callback, Qt::TimerType type);

    int call(TimeMs timeout, ok::base::Fn<void()> callback) {
        return call(timeout, std::move(callback), Timer::DefaultType(timeout));
    }

    void cancel(int callId);

protected:
    void timerEvent(QTimerEvent* e) override;

private:
    std::map<int, ok::base::Fn<void()>> _callbacks;
};

}  // namespace base
