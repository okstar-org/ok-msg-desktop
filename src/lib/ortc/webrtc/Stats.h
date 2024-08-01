#ifndef ORTC_STATS_H
#define ORTC_STATS_H

#include <cstdint>
#include <string>
#include <vector>

namespace lib::ortc {

enum class CallStatsConnectionEndpointType {
    ConnectionEndpointP2P = 0,
    ConnectionEndpointTURN = 1
};

struct CallStatsNetworkRecord {
    int32_t timestamp = 0;
    CallStatsConnectionEndpointType endpointType =
            CallStatsConnectionEndpointType::ConnectionEndpointP2P;
    bool isLowCost = false;
};

struct CallStatsBitrateRecord {
    int32_t timestamp = 0;
    int32_t bitrate = 0;
};

struct CallStats {
    std::string outgoingCodec;
    std::vector<CallStatsNetworkRecord> networkRecords;
    std::vector<CallStatsBitrateRecord> bitrateRecords;
};

}  // namespace lib::ortc

#endif
