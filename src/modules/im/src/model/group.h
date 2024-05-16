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

#include "src/core/contactid.h"
#include "src/core/groupid.h"
#include "src/core/icoregroupquery.h"
#include "src/core/icoreidhandler.h"
#include "src/core/toxpk.h"
#include "src/model/message.h"
#include <QMap>
#include <QObject>
#include <QStringList>



class Group : public Contact
{
    Q_OBJECT
public:
    Group(const GroupId persistentGroupId,
          const QString& name,
          bool isAvGroupchat,
          const QString& selfName,
          ICoreGroupQuery& groupQuery,
          ICoreIdHandler& idHandler);

    bool isAvGroupchat() const;

    int getPeersCount() const;
    void setPeerCount(uint32_t count);

    void addPeer(const GroupOccupant &go);
    const QMap<QString, QString>& getPeerList() const;
    bool peerHasNickname(ToxPk pk);

    void setEventFlag(bool f) override;
    bool getEventFlag() const override;

    void setMentionedFlag(bool f);
    bool getMentionedFlag() const;

    void updateUsername(const QString oldName, const QString newName);

    void setTitle(const QString& author, const QString& newTitle);
    const QString & getTitle()const{return title;};



    void setSelfName(const QString& name);
    QString getSelfName() const;

    void setDesc(const QString& desc_);
    const QString & getDesc() const;

const GroupId& getPersistentId() const {return groupId;};
signals:
    void titleChangedByUser(const QString& title);
    void titleChanged(const QString& author, const QString& title);
    void userJoined(const ToxPk& user, const QString& name);
    void userLeft(const ToxPk& user, const QString& name);
    void peerCountChanged(uint32_t numPeers);
    void peerNameChanged(const QString& oldName, const QString& newName);
    void descChanged(const QString&);

private:
    void stopAudioOfDepartedPeers(const ToxPk& peerPk);

private:
    ICoreGroupQuery& groupQuery;
    ICoreIdHandler& idHandler;
    QString title;
    QString desc;
    uint32_t peerCount;
    QMap<QString, QString> peerDisplayNames;
    bool hasNewMessages;
    bool userWasMentioned;
    const GroupId groupId;
    bool avGroupchat;
};

#endif // GROUP_H
