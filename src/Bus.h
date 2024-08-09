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
// Created by gaojie on 24-8-1.
//

#pragma once

#include <QObject>

class Module;
class Core;
class CoreFile;

namespace ok {

/**
 * 全局系统总线，负责多模块直接交互事件
 */
class Bus : public QObject {
    Q_OBJECT
public:
    Bus(QObject* parent = nullptr);
    ~Bus();
signals:
    void languageChanged(QString locale);
    void moduleCreated(Module* module);
    void coreChanged(Core* core);
    void coreFileChanged(CoreFile* coreFile);
};

}  // namespace ok
