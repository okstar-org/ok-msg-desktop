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

#ifndef POSIXSIGNALNOTIFIER_H
#define POSIXSIGNALNOTIFIER_H

#include <QObject>

class QSocketNotifier;

class PosixSignalNotifier : public QObject {
    Q_OBJECT

public:
    ~PosixSignalNotifier();

    static void watchSignal(int signum);
    static void watchSignals(std::initializer_list<int> signalSet);
    static void watchCommonTerminatingSignals();

    static PosixSignalNotifier& globalInstance();

signals:
    void activated(int signal);

private slots:
    void onSignalReceived();

private:
    PosixSignalNotifier();

private:
    QSocketNotifier* notifier{nullptr};
};

#endif  // POSIXSIGNALNOTIFIER_H
