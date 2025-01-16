
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

#include <map>
#include <memory>
#include <string>
#include <utility>

#include "src/base/logs.h"

#include "PaintItem.h"

namespace module::painter {

class CSharedPaintItemList {
public:
    typedef std::map<ItemId, std::shared_ptr<CPaintItem>> ITEM_MAP;

public:
    CSharedPaintItemList(const std::string& owner) : owner_(owner) {}

    ~CSharedPaintItemList(void) {}

    bool addItem(std::shared_ptr<CPaintItem> item) {
        if (item->itemId().size() <= 0) {
            qWarning() << "item id can't be empty!";
            return false;
        }

        auto s = itemMap_.find(item->itemId());
        if (s != itemMap_.end()) {
            qWarning() << "item is exist!";
            return false;
        }

        std::pair<ITEM_MAP::iterator, bool> ret =
                itemMap_.insert(ITEM_MAP::value_type(item->itemId(), item));
        if (!ret.second) {
            qWarning() << "item id:" << qstring(item->itemId());
            return false;
        }

        return true;
    }

    std::shared_ptr<CPaintItem> findItem(ItemId itemId) {
        ITEM_MAP::iterator it = itemMap_.find(itemId);
        if (it == itemMap_.end()) return std::shared_ptr<CPaintItem>();

        return it->second;
    }

    void removeItem(ItemId itemId) {
        ITEM_MAP::iterator it = itemMap_.find(itemId);
        if (it == itemMap_.end()) return;

        itemMap_.erase(it);
    }

    size_t itemCount(void) {
        return itemMap_.size();
    }

    ITEM_MAP& itemMap() {
        return itemMap_;
    }

private:
    std::string owner_;
    ITEM_MAP itemMap_;
};
}  // namespace module::painter
