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

//
// Created by gaojie on 24-12-22.
//

#include "ok_rtc.h"

namespace lib::ortc {

std::string SignalingStateAsStr(SignalingState state) {
    switch (state) {
        case SignalingState::Stable:
            return "Stable";
        case SignalingState::HaveLocalOffer:
            return "HaveLocalOffer";
        case SignalingState::HaveLocalPrAnswer:
            return "HaveLocalPrAnswer";
        case SignalingState::HaveRemoteOffer:
            return "HaveRemoteOffer";
        case SignalingState::HaveRemotePrAnswer:
            return "HaveRemotePrAnswer";
        case SignalingState::Closed:
            return "Closed";
    }
    return {};
}

std::string IceGatheringStateAsStr(IceGatheringState state) {
    switch (state) {
        case IceGatheringState::New:
            return "New";
        case IceGatheringState::Gathering:
            return "Gathering";
        case IceGatheringState::Complete:
            return "Complete";
    }
    return {};
}
std::string IceConnectionStateAsStr(IceConnectionState state) {
    switch (state) {
        case IceConnectionState::New:
            return "New";
        case IceConnectionState::Closed:
            return "Closed";
        case IceConnectionState::Failed:
            return "Failed";
        case IceConnectionState::Connected:
            return "Connected";
        case IceConnectionState::Disconnected:
            return "Disconnected";
        case IceConnectionState::Checking:
            return "Checking";
        case IceConnectionState::Completed:
            return "Completed";
        case IceConnectionState::Max:
            return "Max";
    }
    return {};
}

std::string PeerConnectionStateAsStr(PeerConnectionState state) {
    switch (state) {
        case PeerConnectionState::New:
            return "New";
        case PeerConnectionState::Connecting:
            return "Connecting";
        case PeerConnectionState::Connected:
            return "Connected";
        case PeerConnectionState::Disconnected:
            return "Disconnected";
        case PeerConnectionState::Failed:
            return "Failed";
        case PeerConnectionState::Closed:
            return "Closed";
    }
    return {};
}

}  // namespace lib::ortc
