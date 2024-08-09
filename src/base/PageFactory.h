
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

#ifndef PAGEFACTORY_H
#define PAGEFACTORY_H
#include "Page.h"
#include <QWidget>
namespace ok::base {

class PageFactory {
public:
    template <typename T>
    static
            // Restrict type T to Page
            typename std::enable_if<true, ok::base::Page>::type  // T -> Page
                    *
                    Create(QWidget* parent = Q_NULLPTR) {
        return new T(parent);
    }
};

} // namespace ok::base

#endif  // PAGEFACTORY_H
