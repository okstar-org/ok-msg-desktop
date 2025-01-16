
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

#include <memory>
#include <stack>

#include "SharedPaintCommand.h"
#include "SharedPaintManagementData.h"

namespace module::classroom {

class CSharedPaintManager;

class CSharedPaintCommandManager {
public:
    static const int DEFAULT_INIT_PLAYBACK_POS = -2;  // unreachable value
    typedef std::map<std::string, std::shared_ptr<CSharedPaintItemList> > ITEM_LIST_MAP;

    CSharedPaintCommandManager(CSharedPaintManager* spManager = nullptr);

    ~CSharedPaintCommandManager();

    void clear(void) {
        clearHistoryItem();
        clearHistoryTask();
        clearHistoryCommand();
    }

    void clearHistoryItem(void) {
        //		////std::recursive_mutex::scoped_lock autolock(mutex_);

        historyItemSet_.clear();
        userItemListMap_.clear();
    }

    void clearHistoryTask(void) {
        currentPlayPos_ = DEFAULT_INIT_PLAYBACK_POS;
        //		////std::recursive_mutex::scoped_lock autolock(mutex_);
        historyTaskList_.clear();
    }

    void clearHistoryCommand(void) {
        //		////std::recursive_mutex::scoped_lock autolock(mutex_);
        while (commandList_.size() > 0) commandList_.pop();
        while (redoCommandList_.size() > 0) redoCommandList_.pop();
    }

    size_t historyItemCount(void) {
        return historyItemSet_.size();
    }

    size_t historyTaskCount(void) {
        return historyTaskList_.size();
    }

    void lock(void) {
        mutex_.lock();
    }

    void unlock(void) {
        mutex_.unlock();
    }

    const ITEM_SET& historyItemSet(void) {
        return historyItemSet_;
    }

    const TASK_LIST& historyTaskList(void) {
        return historyTaskList_;
    }

    size_t generateItemId(void) {
        std::lock_guard<std::recursive_mutex> lock(mutex_);
        return historyItemSet_.size() + 1;
    }

    bool executeTask(std::shared_ptr<CSharedPaintTask> task);

    bool executeCommand(std::shared_ptr<CSharedPaintCommand> command);

    bool redoCommand(void) {
        if (redoCommandList_.size() <= 0) return false;

        std::shared_ptr<CSharedPaintCommand> command = redoCommandList_.top();
        bool ret = command->execute();

        if (ret) {
            //			////std::recursive_mutex::scoped_lock autolock(mutex_);
            commandList_.push(command);
            redoCommandList_.pop();
        }
        return ret;
    }

    bool undoCommand(void) {
        if (commandList_.size() <= 0) return false;

        std::shared_ptr<CSharedPaintCommand> command = commandList_.top();
        bool ret = command->undo();

        if (ret) {
            ////std::recursive_mutex::scoped_lock autolock(mutex_);
            redoCommandList_.push(command);
            commandList_.pop();
        }
        return ret;
    }

    bool isPlaybackMode(void) {
        if (DEFAULT_INIT_PLAYBACK_POS == currentPlayPos_ /* init position */
            || ((int)historyTaskList_.size() - 1 == currentPlayPos_ /* last position */)) {
            return false;
        }
        return true;
    }

    void playbackTo(int position);

    bool addHistoryItem(std::shared_ptr<CPaintItem> item);

    std::shared_ptr<CPaintItem> findItem(const std::string& owner, ItemId itemId) {
        std::lock_guard<std::recursive_mutex> lock(mutex_);

        std::shared_ptr<CSharedPaintItemList> itemList = _findItemList(owner);
        if (!itemList) return std::shared_ptr<CPaintItem>();

        return itemList->findItem(itemId);
    }

    void removeItem(CPaintItem* item) {
        std::lock_guard<std::recursive_mutex> lock(mutex_);

        std::shared_ptr<CSharedPaintItemList> itemList = _findItemList("");
        if (!itemList) return;

        return itemList->removeItem(item->itemId());
    }

    void addPainter(const std::string& userId) {
        ////std::recursive_mutex::scoped_lock autolock(mutex_);
        allowPainters_.insert(userId);
    }

    void setAllowPainterToDraw(const std::string& userId, bool enabled);

    bool isAllowPainterToDraw(const std::string& userId) {
        Q_UNUSED(userId);
        return true;
    }

private:
    void _playforwardTo(int from, int to);

    void _playbackwardTo(int from, int to);

    // not thread safe..
    std::shared_ptr<CSharedPaintItemList> _findItemList(const std::string& owner) {
        ITEM_LIST_MAP::iterator it = userItemListMap_.find(owner);
        if (it == userItemListMap_.end()) return std::shared_ptr<CSharedPaintItemList>();

        return it->second;
    }

protected:
    typedef std::stack<std::shared_ptr<CSharedPaintCommand> > COMMAND_LIST;

    CSharedPaintManager* spManager_;
    int currentPlayPos_;

    TASK_LIST historyTaskList_;
    ITEM_SET historyItemSet_;        // for iterating
    ITEM_LIST_MAP userItemListMap_;  // for searching

    COMMAND_LIST commandList_;
    COMMAND_LIST redoCommandList_;

    std::recursive_mutex mutex_;

    std::set<std::string> allowPainters_;
};
}  // namespace module::classroom
