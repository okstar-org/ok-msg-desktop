#ifndef TGCALLS_LOG_SINK_IMPL_H
#define TGCALLS_LOG_SINK_IMPL_H

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
