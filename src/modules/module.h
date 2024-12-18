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

#include <QObject>
#include <QString>
#include <QWidget>
#include "lib/session/AuthSession.h"

class Profile;

/**
 * 需要保存的信息
 */
typedef struct {
    QByteArray windowGeometry;
} SavedInfo;

/**
 * 模块接口申明
 */
class Module {
public:
    static Module* Create();

    virtual void init(Profile* p) = 0;
    virtual const QString& getName() const = 0;
    virtual QWidget* widget() = 0;

    /**
     * 启动模块
     * @param session
     */
    virtual void start(std::shared_ptr<lib::session::AuthSession> session) = 0;

    /**
     * 停止模块（退出应用程序之前被调用）
     */
    virtual void stop() = 0;

    /**
     * 是否启动
     * @return
     */
    virtual bool isStarted() = 0;

    /**
     * 隐藏
     */
    virtual void hide() = 0;

    /**
     * 用户登出时（但未退出应用），一些清理操作
     */
    virtual void cleanup() = 0;

    /**
     * 模块切换
     */
    virtual void activate() {};

protected:
    /**
     * 保存事件
     */
    virtual void onSave(SavedInfo&) = 0;
};
