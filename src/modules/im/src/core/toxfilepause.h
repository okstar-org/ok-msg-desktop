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

#ifndef TOX_FILE_PAUSE_H
#define TOX_FILE_PAUSE_H

class ToxFilePause {
public:
    void localPause() { localPauseState = true; }

    void localResume() { localPauseState = false; }

    void localPauseToggle() { localPauseState = !localPauseState; }

    void remotePause() { remotePauseState = true; }

    void remoteResume() { remotePauseState = false; }

    void remotePauseToggle() { remotePauseState = !remotePauseState; }

    bool localPaused() const { return localPauseState; }

    bool remotePaused() const { return remotePauseState; }

    bool paused() const { return localPauseState || remotePauseState; }

private:
    bool localPauseState = false;
    bool remotePauseState = false;
};

#endif  // TOX_FILE_PAUSE_H
