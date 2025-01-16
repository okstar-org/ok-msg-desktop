
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

#ifndef TASK_PACKET_BUILDER_H__
#define TASK_PACKET_BUILDER_H__

#include <memory>
#include <string>

#include "src/base/logs.h"

#include "SharedPaintTask.h"
#include "SharedPaintTaskFactory.h"

namespace TaskPacketBuilder {

using namespace module::painter;

class CExecuteTask {
public:
    static std::string make(std::shared_ptr<CSharedPaintTask> task,
                            const std::string* target = nullptr) {
        std::string data = task->serialize();

        return data;
    }

    static std::shared_ptr<CSharedPaintTask> parse(const std::string& body) {
        std::shared_ptr<CSharedPaintTask> task;

        //			try
        //			{
        //				int pos = 0;
        //				std::uint16_t temptype;
        //				pos += CPacketBufferUtil::readInt16( body, pos, temptype, true );

        //				TaskType type = (TaskType)temptype;

        //				task = CSharedPaintTaskFactory::createTask( type );

        //				std::string taskData( (const char *)body.c_str() + pos, body.size() - pos );
        //				if( !task->deserialize( taskData ) )
        //				{
        //					return std::shared_ptr<CSharedPaintTask>();
        //				}
        //			}catch(...)
        //			{
        //				return std::shared_ptr<CSharedPaintTask>();
        //			}

        return task;
    }
};
};  // namespace TaskPacketBuilder

#endif
