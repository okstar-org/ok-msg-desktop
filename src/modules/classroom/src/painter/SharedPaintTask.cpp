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

#include "SharedPaintTask.h"

#include "src/base/logs.h"

#include "SharedPaintCommand.h"
#include "SharedPaintManager.h"
#include "TaskPacketBuilder.h"

//#define DEBUG_PRINT_TASK()    qDebug() << __func__ << "history item cnt : " << cmdMngr_->historyItemCount();

namespace module::classroom {

    static CDefferedCaller gCaller;

    void CSharedPaintTask::sendPacket(void)
    {
        //       //查找数据
        //       std::shared_ptr<CPaintItem> item = cmdMngr_->findItem(data_.owner, data_.itemId);
        //       if(!item){
        //           DEBUG_LOG(("findItem:%1 is be null!").arg(data_.itemId));
        //           return false;
        //       }
    }

    bool CAddItemTask::execute(void)
    {

        //查找数据
        std::shared_ptr<CPaintItem> item = cmdMngr_->findItem(data_.owner, data_.itemId);
        if (!item)
        {
            return false;
        }
        //序列化数据
        //        item->posX();
        //        item->posY();

        //发送数据
        //        sendPacket();

        if (item)
        {
            gCaller.performMainThread(std::bind(&CSharedPaintManager::fireObserver_AddPaintItem, spMngr_, item));
        }

        return true;
    }

    void CAddItemTask::rollback(void)
    {

        std::shared_ptr<CPaintItem> item = cmdMngr_->findItem(data_.owner, data_.itemId);
        if (item)
        {
            gCaller.performMainThread(std::bind(&CSharedPaintManager::fireObserver_RemovePaintItem, spMngr_, item));
        }
    }

    bool CRemoveItemTask::execute(void)
    {

        sendPacket();

        std::shared_ptr<CPaintItem> item = cmdMngr_->findItem(data_.owner, data_.itemId);
        if (item)
        {
            gCaller.performMainThread(std::bind(&CSharedPaintManager::fireObserver_RemovePaintItem, spMngr_, item));
        }

        return true;
    }

    void CRemoveItemTask::rollback(void)
    {

        std::shared_ptr<CPaintItem> item = cmdMngr_->findItem(data_.owner, data_.itemId);
        if (item)
        {
            gCaller.performMainThread(std::bind(&CSharedPaintManager::fireObserver_AddPaintItem, spMngr_, item));
        }
    }

    bool CUpdateItemTask::execute(void)
    {

        sendPacket();

        std::shared_ptr<CPaintItem> item = cmdMngr_->findItem(data_.owner, data_.itemId);
        if (item)
        {
            item->setData(paintData_);
            gCaller.performMainThread(std::bind(&CSharedPaintManager::fireObserver_UpdatePaintItem, spMngr_, item));
        }
        return true;
    }

    void CUpdateItemTask::rollback(void)
    {

        std::shared_ptr<CPaintItem> item = cmdMngr_->findItem(data_.owner, data_.itemId);
        if (item)
        {
            item->setData(prevPaintData_);
            gCaller.performMainThread(std::bind(&CSharedPaintManager::fireObserver_UpdatePaintItem, spMngr_, item));
        }
    }

    bool CMoveItemTask::execute(void)
    {

        sendPacket();

        std::shared_ptr<CPaintItem> item = cmdMngr_->findItem(data_.owner, data_.itemId);
        if (item)
        {
            gCaller.performMainThread(
                std::bind(&CSharedPaintManager::fireObserver_MovePaintItem, spMngr_, item, posX_, posY_));
        }
        return true;
    }

    void CMoveItemTask::rollback(void)
    {

        std::shared_ptr<CPaintItem> item = cmdMngr_->findItem(data_.owner, data_.itemId);
        if (item)
        {
            gCaller.performMainThread(
                std::bind(&CSharedPaintManager::fireObserver_MovePaintItem, spMngr_, item, prevPosX_, prevPosY_));
        }
    }

}
