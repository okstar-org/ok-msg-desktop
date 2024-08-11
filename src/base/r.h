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
namespace ok::base {
#define APPLICATION_VERSION GIT_DESCRIBE
#define APPLICATION_VERSION_ID GIT_VERSION
#define APPLICATION_RELEASE APPLICATION_NAME "-" APPLICATION_VERSION

#define BACKEND_CLOUD_URL "https://cloud.okstar.org.cn/api/open"

#define XMPP_FOCUS_NAME "focus"
#define XMPP_CONFERENCE_FOCUS "focus.meet.chuanshaninfo.com"

#define XMPP_CONF_SERVER_HOST "conference.meet.chuanshaninfo.com"
#define XMPP_PUBSUB_SERVICE "pubsub.meet.chuanshaninfo.com"

#define FILE_PROFILE_EXT ".profile"
#define FILE_INIT_EXT ".ini"
}
