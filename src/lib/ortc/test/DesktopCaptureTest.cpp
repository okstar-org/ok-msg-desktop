 
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

//
// Created by gaojie on 25-2-22.
//
#include <gtest/gtest.h>
#include <iostream>
#include "webrtc/platform/desktop_capturer/DesktopCaptureSourceHelper.h"

TEST(DesktopCaptureTest, BasicAssertions) {
    auto id = "Desktop 0";
    auto source = lib::ortc::DesktopCaptureSourceForKey(id);

    std::cout << "Video device: "<< id << " is valid? " << source.valid() << std::endl;
    if(source.valid()){
        std::cout << "Video device uniqueId is: "<< source.uniqueId() << std::endl;
    }
}
