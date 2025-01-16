
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

#include "DefferedCaller.h"
#include <qdatetime.h>
#include <qtimer.h>
#include <QApplication>
#include <QEvent>
#include <cassert>
#include <thread>

std::thread::id CDefferedCaller::mainThreadId_ = std::this_thread::get_id();

CDefferedCaller::CDefferedCaller(void) : autoDelete_(false) {
    // qDebug() << "CDefferedCaller()" << this;
}

CDefferedCaller::~CDefferedCaller(void) {
    // qDebug() << "~CDefferedCaller()" << this << autoDelete_;
}

bool CDefferedCaller::isMainThread(void) {
    if (mainThreadId_ == std::this_thread::get_id()) return true;
    return false;
}

void CDefferedCaller::performMainThreadAlwaysDeffered(FUNC_TYPE func) {
    mutex_.lock();

    deferredMethods_.push_back(func);

    mutex_.unlock();

    QEvent* evt = new QEvent(QEvent::User);
    moveToThread(QApplication::instance()->thread());
    QApplication::postEvent(this, evt);
}

void CDefferedCaller::performMainThread(FUNC_TYPE func) {
    if (isMainThread()) {
        func();
        return;
    }

    performMainThreadAlwaysDeffered(func);
}

bool CDefferedCaller::performMainThreadAfterMilliseconds(FUNC_TYPE func, int msec) {
    mutex_.lock();

    std::shared_ptr<methodtimer_t> data(new methodtimer_t);
    data->remain_msec = msec;
    data->func = func;
    data->timer_start = false;

    deferredMethodsForTimer_.push_back(data);

    mutex_.unlock();

    QEvent* evt = new QEvent(QEvent::User);
    moveToThread(QApplication::instance()->thread());
    QApplication::postEvent(this, evt);
    return true;
}

void CDefferedCaller::timerEvent(void) {
    mutex_.lock();

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    for (TIMER_LIST::iterator it = deferredMethodsForTimer_.begin();
         it != deferredMethodsForTimer_.end();
         it++) {
        std::shared_ptr<methodtimer_t> data = (*it);
        if ((*it)->timer_start) {
            qint64 diffMsec = now - data->tick;
            assert(diffMsec >= 0);

            data->remain_msec -= diffMsec;
            data->tick = now;
        }
    }

    TIMER_LIST methodsForTimer = deferredMethodsForTimer_;

    mutex_.unlock();

    for (TIMER_LIST::iterator it = methodsForTimer.begin(); it != methodsForTimer.end(); it++) {
        if ((*it)->remain_msec < 10) {
            (*it)->func();

            mutex_.lock();
            TIMER_LIST::iterator itF = std::find(deferredMethodsForTimer_.begin(),
                                                 deferredMethodsForTimer_.end(), *it);
            if (itF != deferredMethodsForTimer_.end()) deferredMethodsForTimer_.erase(itF);
            mutex_.unlock();
        }
    }

    if (deferredMethodsForTimer_.size() <= 0 && autoDelete_) delete this;
}

void CDefferedCaller::customEvent(QEvent* e) {
    mutex_.lock();
    TIMER_LIST methodsForTimer = deferredMethodsForTimer_;
    FUNC_LIST methods = deferredMethods_;
    mutex_.unlock();

    // MUST be lock-free status..
    for (TIMER_LIST::iterator it = methodsForTimer.begin(); it != methodsForTimer.end(); it++) {
        std::shared_ptr<methodtimer_t> data = (*it);
        if (false == data->timer_start) {
            data->tick = QDateTime::currentMSecsSinceEpoch();
            QTimer::singleShot(data->remain_msec, this, SLOT(timerEvent()));
            data->timer_start = true;
        }
    }

    for (FUNC_LIST::iterator it = methods.begin(); it != methods.end(); it++) {
        (*it)();
    }

    if (methodsForTimer.size() != deferredMethodsForTimer_.size()) {
        CDefferedCaller::customEvent(e);  // recursive
        return;
    }

    mutex_.lock();
    if (methods.size() < deferredMethods_.size()) {
        int cnt = methods.size();
        while (--cnt >= 0) deferredMethods_.pop_front();

        mutex_.unlock();

        CDefferedCaller::customEvent(e);  // recursive
        return;
    }

    deferredMethods_.clear();
    mutex_.unlock();

    if (deferredMethodsForTimer_.size() <= 0 && autoDelete_) delete this;
}
