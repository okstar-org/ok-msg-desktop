/*
 * Copyright (c) 2022 船山信息 chuanshaninfo.com
 * The project is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PubL v2 for more details.
 */

#pragma once

#include "messenger.h"
#include <QString>
#include <mucroom.h>
#include <map>
#include <string>

namespace lib {
namespace messenger {

using namespace gloox;

struct IMRoomInfo {
  MUCRoom *room;

  /**
   * 显示项
   * muc#roominfo_<field name>
   * 参考：
   * https://xmpp.org/extensions/xep-0045.html#registrar-formtype-roominfo
   */

  GroupInfo info;

  /**
   * 房间待修改项
   * 1、修改项放入该字段；
   * 2、请求`room->requestRoomConfig()`获取服务器房间所有配置;
   * 3、服务器返回到`handleMUCConfigForm`处理，保存即可；
   *
   * muc#roomconfig_<field name>
   * 参考
   * https://xmpp.org/extensions/xep-0045.html#registrar-formtype-owner
   */
  std::map<std::string, std::string> changes;
};

} // namespace messenger
} // namespace lib
