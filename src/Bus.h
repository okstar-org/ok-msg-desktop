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
class Profile;
class Core;
class CoreFile;
class CoreAV;

namespace ok {

/**
 * 全局系统总线，负责多模块之间事件交互
 */
class Bus : public QObject {
    Q_OBJECT
public:
    explicit Bus(QObject* parent = nullptr);
    ~Bus() {}
signals:
    void languageChanged(QString locale);
    void moduleCreated(Module* module);
    void profileChanged(Profile* profile);
    void coreChanged(Core* core);
    void coreAvChanged(CoreAV* coreAv);
    void coreFileChanged(CoreFile* coreFile);
    void themeColorChanged(int idx, const QString& color);
    //    void fontChanged(const QFont& font);
    //    void fontSizeChanged(int size);
};

}  // namespace ok
