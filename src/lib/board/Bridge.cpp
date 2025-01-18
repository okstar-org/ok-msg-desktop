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

#include "Bridge.h"

#include <QDebug>
#include <utility>

#include "base/basic_types.h"

namespace lib::board {

Bridge::Bridge(messenger::IM* im, BridgeConf conf)
        : im(im), pubsubManager(im->getPubSubManager()), conf(std::move(conf)) {
    pubsubManager->createNode(conf.subdomain, conf.node, nullptr, this);
}

Bridge::~Bridge() {
    qDebug() << __func__;
}

void Bridge::sendEvent(gloox::PubSub::ItemList& items) {
    pubsubManager->publishItem(conf.subdomain, conf.node, items, nullptr, this);
    pubsubManager->subscribe(conf.subdomain, conf.node, this);
}

void Bridge::handleItem(const gloox::JID& service,
                        const std::string& node,
                        const gloox::Tag* entry) {}

void Bridge::handleItems(const std::string& id,
                         const gloox::JID& service,
                         const std::string& node,
                         const gloox::PubSub::ItemList& itemList,
                         const gloox::Error* error) {
    qDebug() << __func__
             << QString("begin args id:%1 service:%2 node:%3 itemList:%4")
                        .arg(qstring(id))
                        .arg(qstring(service.full()))
                        .arg(qstring(node))
                        .arg((itemList.size()));

    if (error) {
        qWarning() << ("error:") << qstring(error->text());
        return;
    }

    for (auto item : itemList) {
        if (item->payload() != nullptr) {
            qDebug() << qstring(item->payload()->name());
        }
    }
}

void Bridge::handleItemPublication(const std::string& id,
                                   const gloox::JID& service,
                                   const std::string& node,
                                   const gloox::PubSub::ItemList& itemList,
                                   const gloox::Error* error) {
    qDebug() << __func__
             << QString("begin args id:%1 service:%2 node:%3 itemList:%4")
                        .arg(qstring(id))
                        .arg(qstring(service.full()))
                        .arg(qstring(node))
                        .arg(itemList.size());
    if (error) {
        qWarning() << qstring(error->text());
        return;
    }

    //    for(auto item: itemList){
    //        DEBUG_LOG(("item.id:%1").arg(qstring(item->id())));
    //        _pubsubManager->requestItems(service, node, item->id(), 100, this);
    //    }
}

void Bridge::handleItemDeletion(const std::string& id,
                                const gloox::JID& service,
                                const std::string& node,
                                const gloox::PubSub::ItemList& itemList,
                                const gloox::Error* error) {}

void Bridge::handleSubscriptionResult(const std::string& id,
                                      const gloox::JID& service,
                                      const std::string& node,
                                      const std::string& sid,
                                      const gloox::JID& jid,
                                      const gloox::PubSub::SubscriptionType subType,
                                      const gloox::Error* error) {
    // DEBUG_LOG(("begin args id:%1 service:%2 node:%3")
    //               .arg(qstring(id))
    //               .arg(qstring(service.full()))
    //               .arg(qstring(node)));

    // if (error != nullptr)
    // {
    //     DEBUG_LOG(("error:%1").arg(qstring(error->text())));
    //     return;
    // }

    // DEBUG_LOG(("jid:%1 SubscriptionType:%2 sid:3")
    //               .arg(qstring(jid.full()))
    //               .arg(subType)
    //               .arg(qstring(sid)));

    // switch (subType)
    // {
    // case gloox::PubSub::SubscriptionNone:
    //     break;
    // case gloox::PubSub::SubscriptionSubscribed:
    //     break;
    // case gloox::PubSub::SubscriptionPending:
    //     break;
    // case gloox::PubSub::SubscriptionUnconfigured:
    //     break;
    // case gloox::PubSub::SubscriptionInvalid:
    //     break;
    // }
}

void Bridge::handleUnsubscriptionResult(const std::string& id,
                                        const gloox::JID& service,
                                        const gloox::Error* error) {
    // DEBUG_LOG(("begin args id:%1 service:%2 error:%3")
    //               .arg(qstring(id))
    //               .arg(qstring(service.full()))
    //               .arg(qstring(error->text())));
}

void Bridge::handleSubscriptionOptions(const std::string& id, const gloox::JID& service,
                                       const gloox::JID& jid, const std::string& node,
                                       const gloox::DataForm* options, const std::string& sid,
                                       const gloox::Error* error) {}

void Bridge::handleSubscriptionOptionsResult(const std::string& id, const gloox::JID& service,
                                             const gloox::JID& jid, const std::string& node,
                                             const std::string& sid, const gloox::Error* error) {}

void Bridge::handleSubscribers(const std::string& id, const gloox::JID& service,
                               const std::string& node, const gloox::PubSub::SubscriptionList& list,
                               const gloox::Error* error) {}

void Bridge::handleSubscribersResult(const std::string& id, const gloox::JID& service,
                                     const std::string& node,
                                     const gloox::PubSub::SubscriberList* list,
                                     const gloox::Error* error) {}

void Bridge::handleAffiliates(const std::string& id, const gloox::JID& service,
                              const std::string& node, const gloox::PubSub::AffiliateList* list,
                              const gloox::Error* error) {}

void Bridge::handleAffiliatesResult(const std::string& id,
                                    const gloox::JID& service,
                                    const std::string& node,
                                    const gloox::PubSub::AffiliateList* list,
                                    const gloox::Error* error) {}

void Bridge::handleNodeConfig(const std::string& id,
                              const gloox::JID& service,
                              const std::string& node,
                              const gloox::DataForm* config,
                              const gloox::Error* error) {}

void Bridge::handleNodeConfigResult(const std::string& id,
                                    const gloox::JID& service,
                                    const std::string& node,
                                    const gloox::Error* error) {}

void Bridge::handleNodeCreation(const std::string& id,
                                const gloox::JID& service,
                                const std::string& node,
                                const gloox::Error* error) {
    // DEBUG_LOG(("begin args id:%1 service:%2 node:%3 error:%4")
    //               .arg(qstring(id))
    //               .arg(qstring(service.full()))
    //               .arg(qstring(node))
    //               .arg(qstring(error->text())));
    // if (error != nullptr)
    // {
    //     return;
    // }
}

void Bridge::handleNodeDeletion(const std::string& id, const gloox::JID& service,
                                const std::string& node, const gloox::Error* error) {}

void Bridge::handleNodePurge(const std::string& id, const gloox::JID& service,
                             const std::string& node, const gloox::Error* error) {}

void Bridge::handleSubscriptions(const std::string& id,
                                 const gloox::JID& service,
                                 const gloox::PubSub::SubscriptionMap& subMap,
                                 const gloox::Error* error) {
    // DEBUG_LOG(("begin args id:%1 service:%2 subMap:%3")
    //               .arg(qstring(id))
    //               .arg(qstring(service.full()))
    //               .arg(subMap.size()));

    // if (error != nullptr)
    // {
    //     DEBUG_LOG(("error:%1").arg(qstring(error->text())));
    //     return;
    // }
}

void Bridge::handleAffiliations(const std::string& id,
                                const gloox::JID& service,
                                const gloox::PubSub::AffiliationMap& affMap,
                                const gloox::Error* error) {}

void Bridge::handleDefaultNodeConfig(const std::string& id,
                                     const gloox::JID& service,
                                     const gloox::DataForm* config,
                                     const gloox::Error* error) {
    // if (error != nullptr)
    // {
    //     DEBUG_LOG(("error:%1").arg(qstring(error->text())));
    //     return;
    // }
    // DEBUG_LOG(("config title:%1").arg(qstring(config->title())));
}

}  // namespace lib::board
