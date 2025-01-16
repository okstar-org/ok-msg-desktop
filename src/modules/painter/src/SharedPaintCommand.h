
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

#include "PaintItem.h"
#include "SharedPaintTask.h"

namespace module::painter {

class CSharedPaintManager;

class CSharedPaintCommandManager;

class CSharedPaintCommand {
public:
    CSharedPaintCommand(std::shared_ptr<CPaintItem> item)
            : cmdMngr_(nullptr), spMngr_(nullptr), item_(item) {}

    virtual ~CSharedPaintCommand(void) {}

private:
    virtual bool execute(void) = 0;

    virtual bool undo(void) = 0;

    std::shared_ptr<CPaintItem> item(void) {
        return item_;
    }

    void setCommandManager(CSharedPaintCommandManager* cmdManager) {
        cmdMngr_ = cmdManager;
    }

    void setSharedPaintManager(CSharedPaintManager* spMngr) {
        spMngr_ = spMngr;
    }

protected:
    friend class CSharedPaintCommandManager;

    CSharedPaintCommandManager* cmdMngr_;
    CSharedPaintManager* spMngr_;
    std::shared_ptr<CPaintItem> item_;
};

class CAddItemCommand : public CSharedPaintCommand {
public:
    CAddItemCommand(std::shared_ptr<CPaintItem> item) : CSharedPaintCommand(item) {}

    ~CAddItemCommand(void) {
        qDebug() << "~CAddItemCommand";
    }

    virtual bool execute(void);

    virtual bool undo(void);
};

class CRemoveItemCommand : public CSharedPaintCommand {
public:
    CRemoveItemCommand(std::shared_ptr<CPaintItem> item) : CSharedPaintCommand(item) {}

    ~CRemoveItemCommand(void) {
        qDebug() << "~CRemoveItemCommand";
    }

    virtual bool execute(void);

    virtual bool undo(void);
};

class CUpdateItemCommand : public CSharedPaintCommand {
public:
    CUpdateItemCommand(std::shared_ptr<CPaintItem> item)
            : CSharedPaintCommand(item), prevData_(item->prevData()), data_(item->data()) {}

    ~CUpdateItemCommand(void) {
        qDebug() << "~CUpdateItemCommand";
    }

    virtual bool execute(void);

    virtual bool undo(void);

private:
    struct SPaintData prevData_;
    struct SPaintData data_;
};

class CMoveItemCommand : public CSharedPaintCommand {
public:
    CMoveItemCommand(std::shared_ptr<CPaintItem> item)
            : CSharedPaintCommand(item)
            , prevX_(item_->prevData().posX)
            , prevY_(item_->prevData().posY)
            , posX_(item->posX())
            , posY_(item->posY()) {}

    ~CMoveItemCommand(void) {
        qDebug() << "~CMoveItemCommand";
    }

    virtual bool execute(void);

    virtual bool undo(void);

private:
    double prevX_;
    double prevY_;
    double posX_;
    double posY_;
};
}  // namespace module::painter
