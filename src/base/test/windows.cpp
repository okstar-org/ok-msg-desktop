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
// Created by gaojie on 25-1-23.
//
#include <gtest/gtest.h>
#include <iostream>
#include <Windows.h>

// Test for Windows screen
TEST(WIN_Screen_Count_Test, BasicAssertions) {
    // Get display screen count
    auto dc = GetSystemMetrics(SM_CMONITORS);
    // Expect greater equal zero.
    std::cout << "Windows Display count:" << dc << std::endl;
    EXPECT_GE(dc, 0);
}