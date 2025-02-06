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

#include "corevideosource.h"
#include "lib/video/videoframe.h"

#include <QDebug>
namespace module::im {

/**
 * @brief CoreVideoSource constructor.
 * @note Only CoreAV should create a CoreVideoSource since
 * only CoreAV can push images to it.
 */
CoreVideoSource::CoreVideoSource() : subscribers{0}, deleteOnClose{false}, stopped{false} {
    qRegisterMetaType<std::shared_ptr<lib::video::OVideoFrame>>(
            "std::shared_ptr<lib::video::OVideoFrame>");
}

/**
 * @brief Makes a copy of the vpx_image_t and emits it as a new VideoFrame.
 * @param vpxframe Frame to copy.
 */
void CoreVideoSource::pushFrame(std::unique_ptr<lib::video::vpx_image_t> vpxframe) {
    if (stopped) {
        qWarning() << "Video was already stopped.";
        return;
    }

    QMutexLocker locker(&biglock);
    auto vframe = lib::video::VideoFrame::convert(id, std::move(vpxframe));
    emit frameAvailable(std::shared_ptr<lib::video::OVideoFrame>(vframe.release()));
}

/**
 * @brief Setup delete on close
 * @param If true, self-delete after the last suscriber is gone
 */
void CoreVideoSource::setDeleteOnClose(bool newstate) {
    QMutexLocker locker(&biglock);
    deleteOnClose = newstate;
}

/**
 * @brief Stopping the source.
 * @see The callers in CoreAV for the rationale
 *
 * Stopping the source will block any pushFrame calls from doing anything
 */
void CoreVideoSource::stopSource() {
    QMutexLocker locker(&biglock);
    stopped = true;
    emit sourceStopped();
}

void CoreVideoSource::restartSource() {
    QMutexLocker locker(&biglock);
    stopped = false;
}
}  // namespace module::im
