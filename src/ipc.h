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

#ifndef IPC_H
#define IPC_H

#include <QMap>
#include <QObject>
#include <QSharedMemory>
#include <QTimer>
#include <ctime>
#include <functional>

namespace ok {

using IPCEventHandler = std::function<bool(const QByteArray&)>;

class IPC : public QObject {
    Q_OBJECT

protected:
    static const int EVENT_TIMER_MS = 3 * 1000;
    static const int EVENT_GC_TIMEOUT = 5;
    static const int EVENT_QUEUE_SIZE = 32;
    static const int OWNERSHIP_TIMEOUT_S = 5;

public:
    IPC(uint32_t profileId = 0, QObject* parent = nullptr);
    ~IPC();

    struct IPCEvent {
        uint32_t dest;
        int32_t sender;
        char name[16];
        char data[128];
        time_t posted;
        time_t processed;
        uint32_t flags;
        bool accepted;
        bool global;
    };

    struct IPCMemory {
        uint64_t globalId;
        time_t lastEvent;
        time_t lastProcessed;
        IPCEvent events[IPC::EVENT_QUEUE_SIZE];
    };

    time_t postEvent(const QString& name, const QByteArray& data = QByteArray(), uint32_t dest = 0);
    bool isCurrentOwner();
    void registerEventHandler(const QString& name, IPCEventHandler handler);
    bool isEventAccepted(time_t time);
    bool waitUntilAccepted(time_t time, int32_t timeout = -1);
    bool isAttached() const;
    bool isAlive();

private:
    IPCMemory* global();
    IPCEvent* fetchEvent();
    bool runEventHandler(IPCEventHandler handler, const QByteArray& arg);
    bool isCurrentOwnerNoLock();

private:
    QTimer timer;
    uint64_t globalId;
    uint32_t profileId;
    QSharedMemory globalMemory;
    QMap<QString, IPCEventHandler> eventHandlers;

public slots:
    void setProfileId(uint32_t profileId);
    void processIpcEvents();
};
}  // namespace ok

#endif  // IPC_H
