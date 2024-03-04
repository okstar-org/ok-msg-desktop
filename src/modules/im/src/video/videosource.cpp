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

#include "videosource.h"

/**
 * @class VideoSource
 * @brief An abstract source of video frames
 *
 * When it has at least one subscriber the source will emit new video frames.
 * Subscribing is recursive, multiple users can subscribe to the same VideoSource.
 */

// Initialize sourceIDs to 0
VideoSource::AtomicIDType VideoSource::sourceIDs{0};
