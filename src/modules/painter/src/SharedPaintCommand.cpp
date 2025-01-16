
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

#include "SharedPaintCommand.h"

#include "PainterView.h"
#include "SharedPaintManager.h"

namespace module::painter {

bool CAddItemCommand::execute(void) {
    std::shared_ptr<CAddItemTask> task(new CAddItemTask(item_->owner(), item_->itemId()));
    return cmdMngr_->executeTask(task);
}

bool CAddItemCommand::undo(void) {
    std::shared_ptr<CRemoveItemTask> task(new CRemoveItemTask(item_->owner(), item_->itemId()));
    return cmdMngr_->executeTask(task);
}

bool CRemoveItemCommand::execute(void) {
    std::shared_ptr<CRemoveItemTask> task(new CRemoveItemTask(item_->owner(), item_->itemId()));
    return cmdMngr_->executeTask(task);
}

bool CRemoveItemCommand::undo(void) {
    std::shared_ptr<CAddItemTask> task(new CAddItemTask(item_->owner(), item_->itemId()));
    return cmdMngr_->executeTask(task);
}

bool CUpdateItemCommand::execute(void) {
    std::shared_ptr<CUpdateItemTask> task(
            new CUpdateItemTask(item_->owner(), item_->itemId(), prevData_, data_));
    return cmdMngr_->executeTask(task);
}

bool CUpdateItemCommand::undo(void) {
    std::shared_ptr<CUpdateItemTask> task(
            new CUpdateItemTask(item_->owner(), item_->itemId(), prevData_, data_));
    return cmdMngr_->executeTask(task);
}

bool CMoveItemCommand::execute(void) {
    std::shared_ptr<CMoveItemTask> task(
            new CMoveItemTask(item_->owner(), item_->itemId(), prevX_, prevY_, posX_, posY_));
    return cmdMngr_->executeTask(task);
}

bool CMoveItemCommand::undo(void) {
    std::shared_ptr<CMoveItemTask> task(
            new CMoveItemTask(item_->owner(), item_->itemId(), prevX_, prevY_, posX_, posY_));
    return cmdMngr_->executeTask(task);
}
}  // namespace module::painter
