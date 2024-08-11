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

#ifndef GROUP_H
#define GROUP_H

#include "contact.h"

#include <QMap>
#include <QObject>
#include <QStringList>
#include "src/core/FriendId.h"
#include "src/core/contactid.h"
#include "src/core/groupid.h"
#include "src/core/icoregroupquery.h"
#include "src/core/icoreidhandler.h"
#include "src/model/message.h"

class Group : public Contact {
    Q_OBJECT
public:
    enum class Role {
        // https://xmpp.org/extensions/xep-0045.html#roles
        None,
        Visitor,
        Participant,
        Moderator,

    };

    enum class Affiliation {
        // https://xmpp.org/extensions/xep-0045.html#affil
        Outcast,  // 被驱逐
        None,
        Owner,
        Admin,
        Member,
    };

    Group(const GroupId persistentGroupId,
          const QString& name,
          bool isAvGroupchat,
          const QString& selfName,
          ICoreGroupQuery& groupQuery,
          ICoreIdHandler& idHandler);

    bool isAvGroupchat() const;

    void addPeer(const GroupOccupant& go);
    int getPeersCount() const;
    void setPeerCount(uint32_t count);
    const QMap<QString, QString>& getPeerList() const;

    bool peerHasNickname(FriendId pk);
    QString getPeerDisplayName(const QString& resource);

    void setEventFlag(bool f) override;
    bool getEventFlag() const override;

    void setMentionedFlag(bool f);
    bool getMentionedFlag() const;

    void updateUsername(const QString oldName, const QString newName);

    void setSubject(const QString& author, const QString& subject);
    const QString& getSubject() const { return subject; };

    void setSelfName(const QString& name);
    QString getSelfName() const;

    void setDesc(const QString& desc_);
    const QString& getDesc() const;

    const Role& getRole() const { return role; }

    const Affiliation& getAffiliation() const { return affiliation; }

    const GroupId& getPersistentId() const { return groupId; };

    void setName(const QString& name);

signals:
    void titleChangedByUser(const QString& title);
    void subjectChanged(const QString& author, const QString& title);
    void userJoined(const FriendId& user, const QString& name);
    void userLeft(const FriendId& user, const QString& name);
    void peerCountChanged(uint32_t numPeers);
    void peerNameChanged(const QString& oldName, const QString& newName);
    void descChanged(const QString&);
    void privilegesChanged(const Role& role, const Affiliation& aff, const QList<int> codes);

private:
    void stopAudioOfDepartedPeers(const FriendId& peerPk);

private:
    ICoreGroupQuery& groupQuery;
    ICoreIdHandler& idHandler;
    QString subject;
    QString desc;
    uint32_t peerCount;
    QMap<QString, QString> peerDisplayNames;
    bool hasNewMessages;
    bool userWasMentioned;
    const GroupId groupId;
    bool avGroupchat;
    Role role{Role::None};
    Affiliation affiliation{Affiliation::None};
    QList<int> statusCodes;
};

#endif  // GROUP_H
