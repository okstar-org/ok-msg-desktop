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

#include <cstddef>
#include <memory>

namespace rtc {
class Thread;
}  // namespace rtc

namespace webrtc {
template <class T> class scoped_refptr;
class SharedModuleThread;
}  // namespace webrtc

namespace lib::ortc {

class Threads {
public:
    virtual ~Threads() = default;
    virtual rtc::Thread* getNetworkThread() = 0;
    virtual rtc::Thread* getMediaThread() = 0;
    virtual rtc::Thread* getWorkerThread() = 0;
    //  virtual rtc::scoped_refptr<webrtc::SharedModuleThread> getSharedModuleThread() = 0;

    // it is not possible to decrease pool size
    static void setPoolSize(size_t size);
    static std::shared_ptr<Threads> getThreads();
};

namespace StaticThreads {
rtc::Thread* getNetworkThread();
rtc::Thread* getMediaThread();
rtc::Thread* getWorkerThread();
// rtc::scoped_refptr<webrtc::SharedModuleThread> getSharedMoudleThread();
std::shared_ptr<Threads>& getThreads();
}  // namespace StaticThreads

};  // namespace lib::ortc
