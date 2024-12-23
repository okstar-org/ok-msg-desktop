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

#ifndef OK_RTC_LOG_SINK_IMPL_H
#define OK_RTC_LOG_SINK_IMPL_H

#include <fstream>
#include "rtc_base/logging.h"

namespace lib::ortc {

class LogSinkImpl final : public rtc::LogSink {
public:
    LogSinkImpl();

    void OnLogMessage(const std::string& msg, rtc::LoggingSeverity severity,
                      const char* tag) override;
    void OnLogMessage(const std::string& message, rtc::LoggingSeverity severity) override;
    void OnLogMessage(const std::string& message) override;

    std::string result() const { return _data.str(); }

private:
    std::ofstream _file;
    std::ostringstream _data;
};

}  // namespace lib::ortc

#endif
