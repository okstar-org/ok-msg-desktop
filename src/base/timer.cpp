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

#include "timer.h"
#include "basic_types.h"

#include <QTimer>
#include <QTimerEvent>
#include <QtCore/QObject>
#include <QtCore/QThread>
#include <algorithm>

namespace base {
namespace {
QObject* TimersAdjuster() {
    static QObject adjuster;
    return &adjuster;
}
}  // namespace

Timer::Timer(QThread* thread, ok::base::Fn<void()> callback)
        : Timer(std::move(callback))  //
{
    moveToThread(thread);
}

Timer::Timer(ok::base::Fn<void()> callback)
        : QObject(nullptr)
        ,  //
        _callback(std::move(callback))
        ,  //
        _type(Qt::PreciseTimer)
        ,                   //
        _adjusted(false) {  //

    setRepeat(Repeat::Interval);
    connect(
            TimersAdjuster(), &QObject::destroyed,  //
            this, [this] { adjust(); },             //
            Qt::QueuedConnection);
}

void Timer::start(TimeMs timeout, Qt::TimerType type, Repeat repeat) {
    cancel();

    _type = type;
    setRepeat(repeat);
    _adjusted = false;
    setTimeout(timeout);
    _timerId = startTimer(_timeout, _type);
    if (_timerId) {
        _next = QTime::currentTime().msec() + _timeout;
    } else {
        _next = 0;
    }
}

void Timer::cancel() {
    if (isActive()) {
        killTimer(ok::base::take(_timerId));
    }
}

TimeMs Timer::remainingTime() const {
    if (!isActive()) {
        return -1;
    }
    auto now = QTime::currentTime().msec();
    return (_next > now) ? (_next - now) : TimeMs(0);
}

void Timer::Adjust() {
    QObject emitter;
    connect(&emitter, &QObject::destroyed, TimersAdjuster(), &QObject::destroyed);
}

void Timer::adjust() {
    auto remaining = remainingTime();
    if (remaining >= 0) {
        cancel();
        _timerId = startTimer(remaining, _type);
        _adjusted = true;
    }
}

void Timer::setTimeout(TimeMs timeout) {
    if (timeout < 0) {
        return;
    }
    _timeout = static_cast<unsigned int>(timeout);
}

int Timer::timeout() const { return _timeout; }

void Timer::timerEvent(QTimerEvent* e) {
    if (repeat() == Repeat::Interval) {
        if (_adjusted) {
            start(_timeout, _type, repeat());
        } else {
            _next = QTime::currentTime().msec() + _timeout;
        }
    } else {
        cancel();
    }

    if (_callback) {
        _callback();
    }
}

int DelayedCallTimer::call(TimeMs timeout, ok::base::Fn<void()> callback, Qt::TimerType type) {
    if (!(timeout >= 0)) {
        return 0;
    }

    if (!callback) {
        return 0;
    }
    auto timerId = QObject::startTimer(static_cast<int>(timeout), type);
    if (timerId) {
        _callbacks.emplace(timerId, std::move(callback));
    }
    return timerId;
}

void DelayedCallTimer::cancel(int callId) {
    if (callId) {
        killTimer(callId);
        _callbacks.erase(callId);
    }
}

void DelayedCallTimer::timerEvent(QTimerEvent* e) {
    auto timerId = e->timerId();
    killTimer(timerId);

    auto it = _callbacks.find(timerId);
    if (it != _callbacks.end()) {
        auto callback = std::move(it->second);
        _callbacks.erase(it);

        callback();
    }
}

}  // namespace base
