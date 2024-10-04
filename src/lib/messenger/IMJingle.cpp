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

#include "IMJingle.h"

#include <QUuid>

#include <capabilities.h>
#include <extdisco.h>
#include <jinglesession.h>

#include "IM.h"
#include "base/logs.h"

namespace lib {
namespace messenger {

using namespace gloox;
using namespace Jingle;
using namespace lib::ortc;

IMJingle::IMJingle(IM* im, QObject* parent) : QObject(parent), _im(im) {
    qDebug() << __func__ << "Creating";

    qRegisterMetaType<std::string>("std::string");
    connect(im, &IM::started, this, &IMJingle::onImStarted);
    qDebug() << __func__ << ("Created");
}

IMJingle::~IMJingle() {
    auto client = _im->getClient();
    client->removeMessageHandler(this);
    qDebug() << __func__ << "Destroyed";
}

void IMJingle::onImStarted() {
    auto client = _im->getClient();
    assert(client);

    client->registerMessageHandler(this);
    client->registerStanzaExtension(new Jingle::JingleMessage());

    auto disco = client->disco();
    // jingle
    disco->addFeature(XMLNS_JINGLE);
    disco->addFeature(XMLNS_JINGLE_MESSAGE);
    disco->addFeature(XMLNS_JINGLE_ERRORS);

}

void IMJingle::handleMessageSession(MessageSession* session) {
    //  session->registerMessageHandler(this);
}

void IMJingle::handleMessage(const Message& msg, MessageSession* session) {
    qDebug() << __func__ << "...";

    /**
     * 处理jingle-message消息
     * https://xmpp.org/extensions/xep-0353.html
     */
    auto jm = msg.findExtension<Jingle::JingleMessage>(ExtJingleMessage);
    if (jm) {
        handleJingleMessage(IMPeerId(msg.from().full()), jm);
    }
}

QString IMJingle::getSessionByFriendId(const QString& friendId) {
    qDebug() << ("getSessionId:%1") << friendId;
    return m_friendSessionMap.value(IMPeerId{friendId}, {});
}

bool IMJingle::handleIq(const IQ& iq) {
    return true;
}

void IMJingle::handleIqID(const IQ& iq, int context) {}


}  // namespace messenger
}  // namespace lib
