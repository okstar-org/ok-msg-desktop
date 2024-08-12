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
class QWidget;
class IAudioControl;

typedef struct {
    QByteArray windowGeometry;
} SavedInfo;

typedef enum {
    MM_Avatar  // 头像
} PayloadType;

typedef struct {
    PayloadType type;
    QByteArray payload;
} ModuleMessage;

class Module {
public:
    static QString Name();
    static Module* Create();

    virtual ~Module(){};
    virtual void init(Profile* p) = 0;
    virtual QString name() = 0;
    virtual QWidget* widget() = 0;
    virtual void start(std::shared_ptr<ok::session::AuthSession> session) = 0;
    virtual bool isStarted() = 0;
    virtual void hide() = 0;
    virtual void onSave(SavedInfo&) = 0;
    virtual void cleanup() = 0;
    virtual void destroy() = 0;
};
