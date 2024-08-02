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

#ifndef IM_CONFERENCE_H
#define IM_CONFERENCE_H

#include <client.h>
#include <jinglesessionhandler.h>
#include <mucroom.h>
#include <memory>
#include "base/timer.h"

namespace lib {
namespace messenger {

using namespace gloox;

class IMConference : public QObject, public DiscoHandler {
    Q_OBJECT
public:
    IMConference(Client* client, Jingle::SessionHandler* sessionHandler);

    void start(const JID& jid);

    void join(const std::string& room);

    void setVideoMute(const std::string& room);

    // Disco handler
    virtual void handleDiscoInfo(const JID& from,     //
                                 const Disco::Info&,  //
                                 int ontext) override;

    virtual void handleDiscoItems(const JID& from,      //
                                  const Disco::Items&,  //
                                  int context) override;

    virtual void handleDiscoError(const JID& from,      //
                                  const gloox::Error*,  //
                                  int context) override;

    //  virtual void onStart(const gloox::Conference *j) override;

private:
    Client* _client;
    Jingle::SessionHandler* _sessionHandler;

    //  std::unique_ptr<gloox::ConferenceManager> _conferenceManager;
    std::shared_ptr<::base::DelayedCallTimer> delayCaller_;

signals:
    void groupListReceived(const QString& peerId);
};
}  // namespace messenger
}  // namespace lib

#endif
