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

#include <memory>

#include "src/audio/audio.h"
#include "src/audio/backend/openal.h"
#include "src/audio/iaudiosettings.h"

/**
 * @brief Select the audio backend
 * @param settings Audio settings to use
 * @return Audio backend selection based on settings
 */
std::unique_ptr<IAudioControl> Audio::makeAudio(IAudioSettings& settings) {
    return std::unique_ptr<IAudioControl>(new OpenAL());
}
