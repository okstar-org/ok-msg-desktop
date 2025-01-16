
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

#include <cassert>
#include <memory>

#include "SharedPaintTask.h"



namespace module::painter {

    class CSharedPaintTaskFactory {
    public:
        static std::shared_ptr<CSharedPaintTask> createTask(TaskType type) {
            std::shared_ptr<CSharedPaintTask> task;
            switch (type) {
                case Task_AddItem:
                    task = std::make_shared<CAddItemTask>();
                    break;
                case Task_RemoveItem:
                    task = std::make_shared<CRemoveItemTask>();
                    break;
                case Task_MoveItem:
                    task = std::make_shared<CMoveItemTask>();
                    break;
                case Task_UpdateItem:
                    task = std::make_shared<CUpdateItemTask>();
                    break;
                default:
                    assert(false && "not supported task type");
            }
            return task;
        }
    };

}
