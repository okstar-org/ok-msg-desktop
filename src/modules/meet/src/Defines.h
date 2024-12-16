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
// Created by gaojie on 24-11-26.
//
#pragma once

namespace module::meet {

/**
 * 会议状态
 */
enum class MeetingState {
    NoMeeting,
    CreatingMeeting,  // 创建中
    Created,          // 已创建
    OnMeeting         // 会议中（自己已经加入）
};

enum class MeetingFrom {
    Create,  // 创建会议
    Join     // 加入会议
};

enum class ParticipantState {
    None,
    Joining,  // 加入中
    Joined    // 已加入
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

}  // namespace module::meet
