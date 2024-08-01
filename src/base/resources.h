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

#include <QDebug>
#include <memory>

#define OK_RESOURCE_LOADER(name)                      \
    class name##Loader {                              \
    public:                                           \
        name##Loader() {                              \
            qDebug() << "loading resource:" << #name; \
            Q_INIT_RESOURCE(name);                    \
        };                                            \
        ~name##Loader() {                             \
            qDebug() << "cleanup resource:" << #name; \
            Q_CLEANUP_RESOURCE(name);                 \
        }                                             \
    };

#define OK_RESOURCE_PTR(name) std::unique_ptr<name##Loader> _ok##name##_ptr

#define OK_RESOURCE_INIT(name) _ok##name##_ptr = std::make_unique<name##Loader>()
