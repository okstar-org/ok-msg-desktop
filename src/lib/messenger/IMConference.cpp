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

#include "IMConference.h"
#include <QTimer>
#include <QUuid>
#include <memory>

#include <capabilities.h>
#include <error.h>
#include "base/logs.h"

namespace lib {
namespace messenger {

using namespace gloox;

IMConference::IMConference(Client* client, Jingle::SessionHandler* sessionHandler)
        : _client(client)
        , _sessionHandler(sessionHandler)
        , delayCaller_((std::make_unique<base::DelayedCallTimer>())) {
    //  _conferenceManager =
    //      std::make_unique<gloox::ConferenceManager>(_client, this);
}

// void IMConference::onStart(const gloox::Conference *j) {
//   auto c = const_cast<gloox::Conference *>(j);
//   qDebug(("room:%1").arg(c->room().c_str()))
//   qDebug(("ready:%1").arg(c->ready()))
//   qDebug(("auth:%1").arg(c->auth()))
//   qDebug(("focusJid:%1").arg(qstring(c->focusJid().full())))
//   this->setVideoMute(c->room());
//
//
// }

/**
 * 发起会议
 * @param jid
 */
void IMConference::start(const JID& jid) {}

void IMConference::join(const std::string& room) {
    //  qDebug(("room:%1").arg(qstring(room)));
    //  std::string room_host = room + "@" + CONFERENCE_HOST;
    //  std::string machineUid = _client->getID();

    //  JID focus(XMPP_CONFERENCE_FOCUS);
    // Conference* _conference2 =  new Conference
    //    (client->client(), focus, this);
    //  _room = std::make_unique<MUCRoom>(_client, JID(room), this, nullptr);
    //  _room->join();
    //  _room->getRoomInfo();
    //  _room->getRoomItems();

    //    static JID focus("focus.meet.chuanshaninfo.com");
    //    IQ iq(IQ::IqType::Set, focus, _client->getID());
    //    iq.addExtension(new Conference(focus, room, machineUid));
    //    _client->send(iq);
    //
    //  delayCaller_->call(5 * 1000, [&]() { this->setVideoMute(room); });
}

void IMConference::setVideoMute(const std::string& room) {
    JID to(room);

    auto cap = new Capabilities();
    cap->setNode("http://jitsi.org/jitsimeet");

    //  Presence p(Presence::PresenceType::Available, to);
    //  p.addExtension(new MUCRoom::MUC);
    //  p.addExtension(new VideoMuted(false));
    //  p.addExtension(cap);
    //  _client->sendPresence(p);
}

// Disco handler
void IMConference::handleDiscoInfo(const JID& from, const Disco::Info& info, int context) {
    QString _from = QString::fromStdString(from.full());

    //  qDebug(("from=%1 context=%2").arg(_from).arg(context));

    //    const StringList features = info.features();
    //    for(auto feature: features){
    //        qDebug(("feature=%1").arg(QString::fromStdString(feature)));
    //    }

    const Disco::IdentityList& identities = info.identities();
    for (auto identity : identities) qDebug() << "identity=" << qstring(identity->name());
}

void IMConference::handleDiscoItems(const JID& from, const Disco::Items& items, int context) {
    QString _from = QString::fromStdString(from.full());
    //  qDebug(("from=%1 context=%2").arg(_from).arg(context));
}

void IMConference::handleDiscoError(const JID& from, const gloox::Error* error, int context) {
    QString _from = qstring(from.full());
    //  qDebug(("from=%1 context=%2 error=%3")
    //                .arg(_from)
    //                .arg(context)
    //                .arg(qstring(error->text())));
}
// Presence Han

}  // namespace messenger
}  // namespace lib
