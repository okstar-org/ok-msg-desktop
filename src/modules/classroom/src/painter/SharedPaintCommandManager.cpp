
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

#include <stdio.h>

#include <functional>

#include "PainterView.h"

#include "DefferedCaller.h"
#include "SharedPaintCommandManager.h"
#include "SharedPaintManager.h"

namespace module::classroom {

static CDefferedCaller gCaller;

CSharedPaintCommandManager::CSharedPaintCommandManager(CSharedPaintManager* spManager)
        : spManager_(spManager), currentPlayPos_(DEFAULT_INIT_PLAYBACK_POS) {}

CSharedPaintCommandManager::~CSharedPaintCommandManager() {}

void CSharedPaintCommandManager::setAllowPainterToDraw(const std::string& userId, bool enabled) {
    if (enabled) {
        if (isAllowPainterToDraw(userId)) return;
        allowPainters_.insert(userId);
    } else {
        if (!isAllowPainterToDraw(userId)) return;
        allowPainters_.erase(userId);
    }

    if (enabled) {
        for (int i = 0; i <= currentPlayPos_; i++) {
            if (historyTaskList_[i]->owner() != userId) continue;
            historyTaskList_[i]->execute();
        }
    } else {
        for (int i = currentPlayPos_; i >= 0; i--) {
            if (historyTaskList_[i]->owner() != userId) continue;
            historyTaskList_[i]->rollback();
        }
    }
}

bool CSharedPaintCommandManager::addHistoryItem(std::shared_ptr<CPaintItem> item) {
    // lock
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    std::shared_ptr<CSharedPaintItemList> itemList;
    ITEM_LIST_MAP::iterator it = userItemListMap_.find(item->owner());
    if (it != userItemListMap_.end()) {
        itemList = it->second;
    } else {
        itemList = std::shared_ptr<CSharedPaintItemList>(new CSharedPaintItemList(item->owner()));
        userItemListMap_.insert(ITEM_LIST_MAP::value_type(item->owner(), itemList));
    }

    auto y = (itemList->addItem(item));
    if (y) historyItemSet_.insert(item);

    return y;
}

bool CSharedPaintCommandManager::executeTask(std::shared_ptr<CSharedPaintTask> task) {
    task->setSharedPaintManager(spManager_);
    task->setCommandManager(this);
    // check current playback position
    {
        // std::recursive_mutex::scoped_lock autolock(mutex_);

        bool playbackWorkingFlag = isPlaybackMode();

        historyTaskList_.push_back(task);

        if (!playbackWorkingFlag) currentPlayPos_ = historyTaskList_.size() - 1;

        gCaller.performMainThread(std::bind(&CSharedPaintManager::fireObserver_AddTask, spManager_,
                                            historyTaskList_.size(), playbackWorkingFlag));

        // now playback working or user not allowed, skip to execute this task.
        if (playbackWorkingFlag || !isAllowPainterToDraw(task->owner())) return true;
    }

    if (!task->execute()) return false;

    return true;
}

bool CSharedPaintCommandManager::executeCommand(std::shared_ptr<CSharedPaintCommand> command) {
    command->setCommandManager(this);
    command->setSharedPaintManager(spManager_);

    bool ret = command->execute();

    if (ret) {
        // std::recursive_mutex::scoped_lock autolock(mutex_);
        commandList_.push(command);

        // redo-list clear
        while (redoCommandList_.size() > 0) redoCommandList_.pop();
    }
    return ret;
}

void CSharedPaintCommandManager::playbackTo(int position) {
    // qDebug() << "playbackTo()" << position;
    if (currentPlayPos_ < position) {
        _playforwardTo(currentPlayPos_, position);
    } else {
        _playbackwardTo(currentPlayPos_, position);
    }

    currentPlayPos_ = position;
}

void CSharedPaintCommandManager::_playforwardTo(int from, int to) {
    if (from < -1 || from >= (int)historyTaskList_.size()) return;
    if (to < 0 || to >= (int)historyTaskList_.size()) return;

    for (int i = from + 1; i <= to; i++) {
        // qDebug() << "_playforwardTo" << i << from << to;
        if (isAllowPainterToDraw(historyTaskList_[i]->owner())) historyTaskList_[i]->execute();
    }
}

void CSharedPaintCommandManager::_playbackwardTo(int from, int to) {
    if (from < 0 || from >= (int)historyTaskList_.size()) return;
    if (to < -1 || to >= (int)historyTaskList_.size()) return;

    for (int i = from; i > to; i--) {
        // qDebug() << "_playbackwardTo" << i << from << to;
        if (isAllowPainterToDraw(historyTaskList_[i]->owner())) historyTaskList_[i]->rollback();
    }
}
}  // namespace module::classroom
