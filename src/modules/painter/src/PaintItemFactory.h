
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

#include "PaintItem.h"

namespace module::painter {

class CPaintItemFactory {
public:
    static std::shared_ptr<CPaintItem> createItem(const ItemId& id, PaintItemType type) {
        std::shared_ptr<CPaintItem> item;
        switch (type) {
            case PT_LINE:
                item = std::make_shared<CLineItem>(id);
                break;
            case PT_FILE:
                item = std::make_shared<CFileItem>(id);
                break;
            case PT_TEXT:
                item = std::make_shared<CTextItem>(id);
                break;
            case PT_IMAGE_FILE:
                item = std::make_shared<CImageFileItem>(id);
                break;
            case PT_IMAGE:
                item = std::make_shared<CImageItem>(id);
                break;
            default:
                assert(false && "not supported item type");
        }
        return item;
    }
};

}  // namespace module::painter
