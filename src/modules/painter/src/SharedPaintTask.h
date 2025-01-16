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

#include <string>
#include <vector>

#include "src/base/logs.h"

#include "PaintItem.h"

namespace module::painter {

class CSharedPaintTask;

class CSharedPaintManager;

class CSharedPaintCommandManager;

typedef std::vector<std::shared_ptr<CSharedPaintTask> > TASK_LIST;

enum TaskType {
    Task_AddItem = 0,
    Task_UpdateItem,
    Task_RemoveItem,
    Task_MoveItem,
};

struct STaskData {
    ItemId itemId;
    std::string owner;
};

class CSharedPaintTask : public std::enable_shared_from_this<CSharedPaintTask> {
public:
    CSharedPaintTask(void) : spMngr_(nullptr) {}

    CSharedPaintTask(const std::string& owner, ItemId itemId) : spMngr_(nullptr) {
        data_.owner = owner;
        data_.itemId = itemId;
    }

    ~CSharedPaintTask(void) {}

public:
    const std::string& owner(void) {
        return data_.owner;
    }

    ItemId itemId(void) {
        return data_.itemId;
    }

    void setCommandManager(CSharedPaintCommandManager* cmdMngr) {
        cmdMngr_ = cmdMngr;
    }

    void setSharedPaintManager(CSharedPaintManager* spMngr) {
        spMngr_ = spMngr;
    }

    void setSendData(bool sendData) {
        sendData_ = sendData;
    }

    void sendPacket(void);

    static bool deserializeBasicData(const std::string& data, struct STaskData& res,
                                     int* readPos = nullptr) {
        return true;
    }

    static std::string serializeBasicData(const struct STaskData& data, int* writePos = nullptr) {
        std::string buf;

        return buf;
    }

    virtual std::string serialize(int* writePos = nullptr) {
        return serializeBasicData(data_, writePos);
    }

    virtual bool deserialize(const std::string& data, int* readPos = nullptr) {
        return deserializeBasicData(data, data_, readPos);
    }

    virtual TaskType type(void) = 0;

    virtual bool execute(void) = 0;

    virtual void rollback(void) = 0;

protected:
    friend class CSharedPaintCommandManager;

    CSharedPaintCommandManager* cmdMngr_;
    CSharedPaintManager* spMngr_;
    struct STaskData data_;
    bool sendData_;
};

//------------------------- CAddItemTask -------------------------

class CAddItemTask : public CSharedPaintTask {
public:
    CAddItemTask(void) : CSharedPaintTask() {}

    CAddItemTask(const std::string& owner, ItemId itemId) : CSharedPaintTask(owner, itemId) {}

    virtual ~CAddItemTask(void) {
        qDebug() << "~CAddItemTask";
    }

    virtual TaskType type(void) {
        return Task_AddItem;
    }

    virtual bool execute(void);

    virtual void rollback(void);
};

class CRemoveItemTask : public CSharedPaintTask {
public:
    CRemoveItemTask(void) : CSharedPaintTask() {}

    CRemoveItemTask(const std::string& owner, ItemId itemId) : CSharedPaintTask(owner, itemId) {}

    virtual ~CRemoveItemTask(void) {
        qDebug() << "~CRemoveItemTask";
    }

    virtual TaskType type(void) {
        return Task_RemoveItem;
    }

    virtual bool execute(void);

    virtual void rollback(void);
};

class CUpdateItemTask : public CSharedPaintTask {
public:
    CUpdateItemTask(void) : CSharedPaintTask() {}

    CUpdateItemTask(const std::string& owner, ItemId itemId, const struct SPaintData& prevData,
                    const struct SPaintData& data)
            : CSharedPaintTask(owner, itemId), prevPaintData_(prevData), paintData_(data) {}

    virtual ~CUpdateItemTask(void) {
        qDebug() << "~CUpdateItemTask";
    }

    virtual TaskType type(void) {
        return Task_UpdateItem;
    }

    virtual bool execute(void);

    virtual void rollback(void);

    virtual std::string serialize(int* writePos = nullptr) {
        std::string data;

        data = CSharedPaintTask::serialize();
        data += CPaintItem::serializeBasicData(prevPaintData_);
        data += CPaintItem::serializeBasicData(paintData_);
        return data;
    }

    virtual bool deserialize(const std::string& data, int* readPos = nullptr) {
        return true;
    }

private:
    struct SPaintData prevPaintData_;
    struct SPaintData paintData_;
};

class CMoveItemTask : public CSharedPaintTask {
public:
    CMoveItemTask(void) : CSharedPaintTask() {}

    CMoveItemTask(const std::string& owner, ItemId itemId, double prevPosX, double prevPosY,
                  double posX, double posY)
            : CSharedPaintTask(owner, itemId)
            , prevPosX_(prevPosX)
            , prevPosY_(prevPosY)
            , posX_(posX)
            , posY_(posY) {}

    virtual ~CMoveItemTask(void) {
        qDebug() << "~CMoveItemCommand";
    }

    virtual TaskType type(void) {
        return Task_MoveItem;
    }

    virtual bool execute(void);

    virtual void rollback(void);

    virtual std::string serialize(int* writePos = nullptr) {
        std::string data;
        return data;
    }

    virtual bool deserialize(const std::string& data, int* readPos = nullptr) {
        return true;
    }

private:
    double prevPosX_;
    double prevPosY_;
    double posX_;
    double posY_;
};
}  // namespace module::painter
