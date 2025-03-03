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
// Created by gaojie on 25-1-17.
//
#pragma once
#include <QString>

namespace module::classroom {
enum class RoomState {
    None,
    Creating,  // 创建中
    Created,   // 已创建
    Meeting,   // 会议中（自己已经加入）
    Quited,    // 已退出
};

/**
 * 分享信息
 */
struct Share {
    // 会议编号
    QString no;
    // 会议名称
    QString name;
};

}  // namespace module::classroom
