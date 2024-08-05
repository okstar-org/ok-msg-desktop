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

#ifndef COREVIDEOSOURCE_H
#define COREVIDEOSOURCE_H

#include <QMutex>
#include <atomic>
#include "videosource.h"

struct vpx_image;

class CoreVideoSource : public VideoSource {
    Q_OBJECT
public:
    CoreVideoSource();

    // VideoSource interface
    virtual void subscribe() override;
    virtual void unsubscribe() override;

private:
    void pushFrame(const vpx_image* frame);
    void setDeleteOnClose(bool newstate);

    void stopSource();
    void restartSource();

private:
    std::atomic_int subscribers;
    std::atomic_bool deleteOnClose;
    QMutex biglock;
    std::atomic_bool stopped;

    friend class CoreAV;
    friend class ToxFriendCall;
};

#endif  // COREVIDEOSOURCE_H
