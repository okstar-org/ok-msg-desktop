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
// Created by gaojie on 25-1-17.
//

#pragma once

#include <pubsubmanager.h>
#include <pubsubresulthandler.h>
#include <memory>
#include "lib/messenger/IM.h"

namespace lib::board {

struct BridgeConf {
    std::string node;
    std::string subdomain;
};

class Bridge : public gloox::PubSub::ResultHandler {
public:
    explicit Bridge(messenger::IM* im, BridgeConf conf);
    ~Bridge();

    void sendEvent(gloox::PubSub::ItemList& items);

protected:
    // PubSub
    void handleItem(const gloox::JID& service,
                    const std::string& node,
                    const gloox::Tag* entry) override;

    void handleItems(const std::string& id,
                     const gloox::JID& service,
                     const std::string& node,
                     const gloox::PubSub::ItemList& itemList,
                     const gloox::Error* error = nullptr) override;

    void handleItemPublication(const std::string& id,
                               const gloox::JID& service,
                               const std::string& node,
                               const gloox::PubSub::ItemList& itemList,
                               const gloox::Error* error = nullptr) override;

    void handleItemDeletion(const std::string& id,
                            const gloox::JID& service,
                            const std::string& node,
                            const gloox::PubSub::ItemList& itemList,
                            const gloox::Error* error = nullptr) override;

    void handleSubscriptionResult(const std::string& id,
                                  const gloox::JID& service,
                                  const std::string& node,
                                  const std::string& sid,
                                  const gloox::JID& jid,
                                  const gloox::PubSub::SubscriptionType subType,
                                  const gloox::Error* error = nullptr) override;

    void handleUnsubscriptionResult(const std::string& id,
                                    const gloox::JID& service,
                                    const gloox::Error* error = nullptr) override;

    void handleSubscriptionOptions(const std::string& id,
                                   const gloox::JID& service,
                                   const gloox::JID& jid,
                                   const std::string& node,
                                   const gloox::DataForm* options,
                                   const std::string& sid,
                                   const gloox::Error* error = nullptr) override;

    void handleSubscriptionOptionsResult(const std::string& id,
                                         const gloox::JID& service,
                                         const gloox::JID& jid,
                                         const std::string& node,
                                         const std::string& sid,
                                         const gloox::Error* error = nullptr) override;

    void handleSubscribers(const std::string& id,
                           const gloox::JID& service,
                           const std::string& node,
                           const gloox::PubSub::SubscriptionList& list,
                           const gloox::Error* error = nullptr) override;

    void handleSubscribersResult(const std::string& id,
                                 const gloox::JID& service,
                                 const std::string& node,
                                 const gloox::PubSub::SubscriberList* list,
                                 const gloox::Error* error = nullptr) override;

    void handleAffiliates(const std::string& id,
                          const gloox::JID& service,
                          const std::string& node,
                          const gloox::PubSub::AffiliateList* list,
                          const gloox::Error* error = nullptr) override;

    void handleAffiliatesResult(const std::string& id,
                                const gloox::JID& service,
                                const std::string& node,
                                const gloox::PubSub::AffiliateList* list,
                                const gloox::Error* error = nullptr) override;

    void handleNodeConfig(const std::string& id,
                          const gloox::JID& service,
                          const std::string& node,
                          const gloox::DataForm* config,
                          const gloox::Error* error = nullptr) override;

    void handleNodeConfigResult(const std::string& id,
                                const gloox::JID& service,
                                const std::string& node,
                                const gloox::Error* error = nullptr) override;

    void handleNodeCreation(const std::string& id,
                            const gloox::JID& service,
                            const std::string& node,
                            const gloox::Error* error = nullptr) override;

    void handleNodeDeletion(const std::string& id,
                            const gloox::JID& service,
                            const std::string& node,
                            const gloox::Error* error = nullptr) override;

    void handleNodePurge(const std::string& id,
                         const gloox::JID& service,
                         const std::string& node,
                         const gloox::Error* error = nullptr) override;

    void handleSubscriptions(const std::string& id,
                             const gloox::JID& service,
                             const gloox::PubSub::SubscriptionMap& subMap,
                             const gloox::Error* error = nullptr) override;

    void handleAffiliations(const std::string& id,
                            const gloox::JID& service,
                            const gloox::PubSub::AffiliationMap& affMap,
                            const gloox::Error* error = nullptr) override;

    void handleDefaultNodeConfig(const std::string& id,
                                 const gloox::JID& service,
                                 const gloox::DataForm* config,
                                 const gloox::Error* error = nullptr) override;

private:
    BridgeConf conf;
    gloox::JID jid;
    gloox::PubSub::Manager* pubsubManager;
    messenger::IM* im;
};
}  // namespace lib::board
