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

#include <memory>
#include <string>

namespace lib {
namespace ortc {

struct RendererImage {
  size_t width_;
  size_t height_;
  uint8_t *y;      //
  uint8_t *u;      //
  uint8_t *v;      //
  int32_t ystride; //
  int32_t ustride; //
  int32_t vstride; //
};

class OkRTCRenderer {
public:
  virtual void onRender(const std::string &friendId, RendererImage image) = 0;
};

} // namespace ortc
} // namespace lib
