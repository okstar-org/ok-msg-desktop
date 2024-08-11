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

//
// Created by gaojie on 24-7-31.
//

#pragma once

#include <QFrame>

class Module;

namespace UI {

/**
 * 代表一个菜单控件(左侧一级业务功能)，内部维护菜单内所有关联（模块、页面集）
 */
class OMenuWidget : public QFrame {
    Q_OBJECT
public:
    explicit OMenuWidget(QWidget* parent = nullptr);
    ~OMenuWidget();

    void setModule(Module* module_){
        module = module_;
    }

    Module* getModule()const{
        return module;
    }

private:
    Module* module = nullptr;
};
}  // namespace UI
