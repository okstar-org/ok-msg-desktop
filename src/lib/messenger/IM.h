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
#pragma once


#include "IMFromHostHandler.h"
#include "IMGroup.h"

#include "base/compatiblerecursivemutex.h"
#include "lib/messenger/IMFriend.h"
#include "lib/messenger/IMMessage.h"
#include "messenger.h"

#include <memory>
#include <range/v3/all.hpp>
#include <set>

#include <bookmarkhandler.h>
#include <bookmarkstorage.h>
#include <carbons.h>
#include <chatstatefilter.h>
#include <chatstatehandler.h>
#include <client.h>
#include <connectionlistener.h>
#include <extdisco.h>
#include <forward.h>
#include <incominghandler.h>
#include <iqhandler.h>
#include <jinglecontent.h>
#include <jinglemessage.h>
#include <jinglesession.h>
#include <jinglesessionhandler.h>
#include <jinglesessionmanager.h>
#include <loghandler.h>
#include <logsink.h>
#include <messageeventfilter.h>
#include <messageeventhandler.h>
#include <messagesessionhandler.h>
#include <mucroom.h>
#include <mucroomconfighandler.h>
#include <mucroomhandler.h>
#include <nativebookmarkhandler.h>
#include <nativebookmarkstorage.h>
#include <pinghandler.h>
#include <presence.h>
#include <pubsub.h>
#include <pubsubevent.h>
#include <pubsubitem.h>
#include <pubsubmanager.h>
#include <pubsubresulthandler.h>
#include <registration.h>
#include <registrationhandler.h>
#include <rosterlistener.h>
#include <rostermanager.h>
#include <vcardhandler.h>
#include <vcardmanager.h>
#include <QDomElement>
#include <QMutex>
#include <QString>

namespace gloox {
class MUCRoom;
class Message;
};  // namespace gloox

namespace lib::ortc {
enum class JingleCallType;
}

namespace lib::messenger {

class IMSessionHandler {
public:
    virtual bool doSessionInitiate(gloox::Jingle::Session* session,        //
                                   const gloox::Jingle::Session::Jingle*,  //
                                   const IMPeerId&) = 0;

    virtual bool doSessionTerminate(gloox::Jingle::Session* session,        //
                                    const gloox::Jingle::Session::Jingle*,  //
                                    const IMPeerId&) = 0;

    virtual bool doSessionAccept(gloox::Jingle::Session* session,               //
                                 const gloox::Jingle::Session::Jingle* jingle,  //
                                 const IMPeerId& peerId) = 0;

    virtual bool doSessionInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual bool doContentAdd(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual bool doContentRemove(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual bool doContentModify(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual bool doContentAccept(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual bool doContentReject(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual bool doTransportInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual bool doTransportAccept(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual bool doTransportReject(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual bool doTransportReplace(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual bool doSecurityInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual bool doDescriptionInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual bool doSourceAdd(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
    virtual bool doInvalidAction(const gloox::Jingle::Session::Jingle*, const IMPeerId&) = 0;
};

struct IMRoomInfo {
    gloox::MUCRoom* room;

    /**
     * 显示项
     * muc#roominfo_<field name>
     * 参考：
     * https://xmpp.org/extensions/xep-0045.html#registrar-formtype-roominfo
     */

    IMGroup info;

    /**
     * 房间待修改项
     * 1、修改项放入该字段；
     * 2、请求`room->requestRoomConfig()`获取服务器房间所有配置;
     * 3、服务器返回到`handleMUCConfigForm`处理，保存即可；
     *
     * muc#roomconfig_<field name>
     * 参考
     * https://xmpp.org/extensions/xep-0045.html#registrar-formtype-owner
     */
    std::map<std::string, std::string> changes;
};

class IM : public QObject,
           public gloox::ConnectionListener,
           public gloox::PingHandler,
           public gloox::RegistrationHandler,
           public gloox::IncomingHandler,
           public gloox::PubSub::ResultHandler,
           public gloox::RosterListener,
           public gloox::MUCRoomHandler,
           public gloox::MUCRoomConfigHandler,
           public gloox::MessageSessionHandler,
           public gloox::MessageHandler,
           public gloox::MessageEventHandler,
           public gloox::ChatStateHandler,
           public gloox::DiscoHandler,
           public gloox::DiscoNodeHandler,
           public gloox::PresenceHandler,
           public gloox::LogHandler,
           public gloox::VCardHandler,
           public gloox::TagHandler,
           public gloox::IqHandler,
           public gloox::BookmarkHandler,
           public gloox::NativeBookmarkHandler,
           public gloox::Jingle::SessionHandler {
    Q_OBJECT
public:
    explicit IM(QString host, QString user, QString pwd, QStringList features);
    ~IM() override;

    inline static IMMessage fromXMsg(MsgType type, const gloox::Message& msg);

    std::unique_ptr<gloox::Client> makeClient();

    void setNickname(const QString& nickname);
    QString getNickname();

    void setAvatar(const QByteArray& avatar);
    void changePassword(const QString& password);
    IMContactId getSelfId();
    IMPeerId getSelfPeerId();
    QString getSelfUsername();

    // External Service Discovery
    const std::vector<ortc::IceServer>& getExternalServiceDiscovery() const {
        return mExtSrvDiscos;
    }

    /**
     * fetchVCard
     */
    void fetchFriendVCard(const QString& friendId);
    IMStatus getFriendStatus(const QString& friendId);
    void requestFriendNickname(const gloox::JID& friendId);

    /**
     * send
     */
    void sendPresence();
    void sendPresence(const gloox::JID& to, gloox::Presence::PresenceType type);
    void sendReceiptReceived(const QString& id, QString receiptNum);

    /**
     * 服务发现
     */
    void enableDiscoManager();

    void requestBookmarks();

    void requestVCards();

    /**
     * 朋友相关
     */
    gloox::RosterManager* enableRosterManager();

    void addFriend(const gloox::JID& jid, const QString& nick, const QString& msg);
    bool removeFriend(const gloox::JID& jid);

    void acceptFriendRequest(const QString&);
    void rejectFriendRequest(const QString&);

    size_t getRosterCount();
    void getRosterList(std::list<IMFriend>&);

    void setFriendAlias(const gloox::JID& jid, const std::string& alias);

    // gloox log
    void handleLog(gloox::LogLevel level, gloox::LogArea area, const std::string& message) override;

    inline const gloox::JID& self() const {
        return _client->jid();
    }

    inline gloox::Client* getClient() const {
        return _client.get();
    }

    QDomDocument buildMessage(const QString& to, const QString& msg, const QString& id);

    bool sendTo(const QString& to, const QString& msg, const QString& id);

    /**
     * 群组相关
     * @param groupId
     * @param nick
     */
    void setRoomSubject(const QString& groupId, const std::string& nick);
    void setRoomName(const QString& groupId, const std::string& name);
    void setRoomAlias(const QString& groupId, const std::string& alias);
    void setRoomDesc(const QString& groupId, const std::string& desc);
    bool inviteToRoom(const gloox::JID& roomJid, const gloox::JID& peerId);
    bool leaveGroup(const QString& groupId);
    bool destroyGroup(const QString& groupId);

    /**
     * @brief sendToRoom
     * @param to
     * @param msg
     * @param id 设置消息ID
     */
    bool sendToRoom(const QString& to, const QString& msg, const QString& id = "");

    void joinRoom(const QString& jid);

    void createRoom(const gloox::JID& jid, const std::string& password = "");

    const IMRoomInfo* findRoom(const QString& groupId) const;

    void start();
    bool isStarted() const;

    void doConnect();

    void doDisconnect();

    void stop();

    // 2-群组列表
    void loadGroupList();
    // 3-用户信息
    void loadRosterInfo();
    // 4-加入群聊
    void joinRooms();

    void send(const QString& xml);

    /**
     * 获取第一个在线终端resource
     * @param bare
     * @return
     */
    std::string getOnlineResource(const std::string& bare);

    /**
     * 获取全部在线终端
     * @param bare
     * @return
     */
    std::set<std::string> getOnlineResources(const std::string& bare);
    void updateOnlineStatus(const std::string& bare, const std::string& resource,
                            gloox::Presence::PresenceType presenceType);

    [[nodiscard]] gloox::JID wrapJid(const QString& f) const;

    [[nodiscard]] gloox::JID wrapRoomJid(const QString& group) const;
    void sendChatState(const QString& to, gloox::ChatStateType state);

    void makeId(QString& id);

    //  ExtDisco &extDisco() { return mExtDisco; }

    inline gloox::Jingle::SessionManager* sessionManager() const {
        return _sessionManager.get();
    }

    gloox::Jingle::Session* createSession(const gloox::JID& jid, const std::string& sId,
                                          IMSessionHandler* h);

    void removeSession(gloox::Jingle::Session* s);

    void addSessionHandler(IMSessionHandler* h);
    void removeSessionHandler(IMSessionHandler* h);
    void removeSelfHandler(SelfHandler* h);

    void addFromHostHandler(const std::string& from, IMFromHostHandler* h);
    void clearFromHostHandler();

    void addFriendHandler(FriendHandler*);
    void addSelfHandler(SelfHandler *);
    void addGroupHandler(GroupHandler *);
    void addIMHandler(IMHandler *);

protected:

    virtual void handleBookmarks(const gloox::BMConferenceList& cList) override;


    /**
     * ping handler
     */
    virtual void handlePing(const gloox::PingHandler::PingType type,
                            const std::string& body) override;


    /**
     * incoming handler
     */
    virtual void handleIncoming(gloox::Tag* tag) override;

    /**
     * iq handlers
     * @param iq
     * @return
     */
    bool handleIq(const gloox::IQ& iq) override;
    void handleIqID(const gloox::IQ& iq, int context) override;

    /**
     * tag handlers
     */
    void handleTag(gloox::Tag* tag) override;

    /**
     * connect handlers
     */
    void onConnect() override;
    void onDisconnect(gloox::ConnectionError e) override;
    bool onTLSConnect(const gloox::CertInfo& info) override;

    /**
     * Registration
     */
    virtual void handleRegistrationFields(const gloox::JID& from, int fields,
                                          std::string instructions) override;

    virtual void handleAlreadyRegistered(const gloox::JID& from) override;

    virtual void handleRegistrationResult(const gloox::JID& from,               //
                                          gloox::RegistrationResult regResult,  //
                                          const gloox::Error* error) override;

    virtual void handleDataForm(const gloox::JID& from, const gloox::DataForm& form) override;

    virtual void handleOOB(const gloox::JID& from, const gloox::OOB& oob) override;

    /**
     * vCard
     * @param jid
     * @param vch
     */
    void handleVCard(const gloox::JID& jid, const gloox::VCard* vcard) override;

    void handleVCardResult(VCardContext context, const gloox::JID& jid,
                           gloox::StanzaError error = gloox::StanzaErrorUndefined) override;

    /**
     * RosterListener
     * @param msg
     * @param session
     */
    void handleItemAdded(const gloox::JID&) override;
    void handleItemSubscribed(const gloox::JID& jid) override;
    void handleItemRemoved(const gloox::JID& jid) override;
    void handleItemUpdated(const gloox::JID& jid) override;
    void handleItemUnsubscribed(const gloox::JID& jid) override;
    void handleRoster(const gloox::Roster& roster) override;
    void handleRosterPresence(const gloox::RosterItem& item, const std::string& resource,
                              gloox::Presence::PresenceType presence,
                              const std::string& msg) override;
    void handleSelfPresence(const gloox::RosterItem& item, const std::string& resource,
                            const gloox::Presence::PresenceType presence,
                            const std::string& msg) override;

    bool handleSubscriptionRequest(const gloox::JID& jid, const std::string& msg) override;

    bool handleUnsubscriptionRequest(const gloox::JID& jid, const std::string& msg) override;

    void handleNonrosterPresence(const gloox::Presence& presence) override;

    void handleRosterError(const gloox::IQ& iq) override;

    void handleRosterItemExchange(const gloox::JID& from, const gloox::RosterX* items) override;

    // MUC config
    void handleMUCConfigList(gloox::MUCRoom* room, const gloox::MUCListItemList& items,
                             gloox::MUCOperation operation) override;

    void handleMUCConfigForm(gloox::MUCRoom* room, const gloox::DataForm& form) override;

    void handleMUCConfigResult(gloox::MUCRoom* room, bool success,
                               gloox::MUCOperation operation) override;

    void handleMUCRequest(gloox::MUCRoom* room, const gloox::DataForm& form) override;

    // MessageSessionHandler
    void handleMessage(const gloox::Message& msg,
                       gloox::MessageSession* session = nullptr) override;
    void handleMessageSession(gloox::MessageSession* session) override;

    // MessageEventHandler
    void handleMessageEvent(const gloox::JID& from, const gloox::MessageEvent* event) override;

    void handleChatState(const gloox::JID& from, gloox::ChatStateType state) override;

    // MUC handler
    void handleMUCParticipantPresence(gloox::MUCRoom* room,
                                      const gloox::MUCRoomParticipant participant,
                                      const gloox::Presence& presence) override;

    void handleMUCMessage(gloox::MUCRoom* room, const gloox::Message& msg, bool priv) override;

    bool handleMUCRoomCreation(gloox::MUCRoom* room) override;

    void handleMUCSubject(gloox::MUCRoom* room, const std::string& nick,
                          const std::string& subject) override;

    void handleMUCInviteDecline(gloox::MUCRoom* room, const gloox::JID& invitee,
                                const std::string& reason) override;

    void handleMUCError(gloox::MUCRoom* room, gloox::StanzaError error) override;

    void handleMUCInfo(gloox::MUCRoom* room, int features, const std::string& name,
                       const gloox::DataForm* infoForm) override;

    void handleMUCItems(gloox::MUCRoom* room, const gloox::Disco::ItemList& items) override;

    // Disco handler
    void handleDiscoInfo(const gloox::JID& from, const gloox::Disco::Info&, int ontext) override;

    void handleDiscoItems(const gloox::JID& from, const gloox::Disco::Items&, int context) override;

    void handleDiscoError(const gloox::JID& from, const gloox::Error*, int context) override;
    // DiscoNodeHandler
    virtual gloox::StringList handleDiscoNodeFeatures(const gloox::JID& from,
                                                      const std::string& node) override;

    virtual gloox::Disco::IdentityList handleDiscoNodeIdentities(const gloox::JID& from,
                                                                 const std::string& node) override;

    virtual gloox::Disco::ItemList handleDiscoNodeItems(
            const gloox::JID& from, const gloox::JID& to,
            const std::string& node = gloox::EmptyString) override;

    // Presence handler
    void handlePresence(const gloox::Presence& presence) override;


    void handleItem(const gloox::JID& service, const std::string& node,
                    const gloox::Tag* entry) override;


    void handleItems(const std::string& id, const gloox::JID& service, const std::string& node,
                     const gloox::PubSub::ItemList& itemList,
                     const gloox::Error* error = 0) override;


    void handleItemPublication(const std::string& id, const gloox::JID& service,
                               const std::string& node, const gloox::PubSub::ItemList& itemList,
                               const gloox::Error* error = 0) override;

    void handleItemDeletion(const std::string& id, const gloox::JID& service,
                            const std::string& node, const gloox::PubSub::ItemList& itemList,
                            const gloox::Error* error = 0) override;


    void handleSubscriptionResult(const std::string& id, const gloox::JID& service,
                                  const std::string& node, const std::string& sid,
                                  const gloox::JID& jid,
                                  const gloox::PubSub::SubscriptionType subType,
                                  const gloox::Error* error = 0) override;

    void handleUnsubscriptionResult(const std::string& id, const gloox::JID& service,
                                    const gloox::Error* error = 0) override;


    void handleSubscriptionOptions(const std::string& id, const gloox::JID& service,
                                   const gloox::JID& jid, const std::string& node,
                                   const gloox::DataForm* options,
                                   const std::string& sid = gloox::EmptyString,
                                   const gloox::Error* error = 0) override;


    void handleSubscriptionOptionsResult(const std::string& id, const gloox::JID& service,
                                         const gloox::JID& jid, const std::string& node,
                                         const std::string& sid = gloox::EmptyString,
                                         const gloox::Error* error = 0) override;

    void handleSubscribers(const std::string& id, const gloox::JID& service,
                           const std::string& node, const gloox::PubSub::SubscriptionList& list,
                           const gloox::Error* error = 0) override;

    void handleSubscribersResult(const std::string& id, const gloox::JID& service,
                                 const std::string& node, const gloox::PubSub::SubscriberList* list,
                                 const gloox::Error* error = 0) override;


    void handleAffiliates(const std::string& id, const gloox::JID& service, const std::string& node,
                          const gloox::PubSub::AffiliateList* list,
                          const gloox::Error* error = 0) override;

    void handleAffiliatesResult(const std::string& id, const gloox::JID& service,
                                const std::string& node, const gloox::PubSub::AffiliateList* list,
                                const gloox::Error* error = 0) override;

    void handleNodeConfig(const std::string& id, const gloox::JID& service, const std::string& node,
                          const gloox::DataForm* config, const gloox::Error* error = 0) override;


    void handleNodeConfigResult(const std::string& id, const gloox::JID& service,
                                const std::string& node, const gloox::Error* error = 0) override;

    void handleNodeCreation(const std::string& id,
                            const gloox::JID& service,
                            const std::string& node,
                            const gloox::Error* error = 0) override;


    void handleNodeDeletion(const std::string& id,
                            const gloox::JID& service,
                            const std::string& node,
                            const gloox::Error* error = 0) override;

    void handleNodePurge(const std::string& id,
                         const gloox::JID& service,
                         const std::string& node,
                         const gloox::Error* error = 0) override;


    void handleSubscriptions(const std::string& id, const gloox::JID& service,
                             const gloox::PubSub::SubscriptionMap& subMap,
                             const gloox::Error* error = 0) override;

    void handleAffiliations(const std::string& id, const gloox::JID& service,
                            const gloox::PubSub::AffiliationMap& affMap,
                            const gloox::Error* error = 0) override;

    void handleDefaultNodeConfig(const std::string& id,          //
                                 const gloox::JID& service,      //
                                 const gloox::DataForm* config,  //
                                 const gloox::Error* error = 0) override;

    void handleBookmarks(const gloox::BookmarkList& bList,  //
                         const gloox::ConferenceList& cList) override;

    void doPubSubEvent(const gloox::PubSub::Event* pse, const gloox::Message& msg,
                       QString& friendId);
    void doMessageHeadline(const gloox::Message& msg, QString& friendId, const QString& body);
    void doMessageChat(const gloox::Message& msg, QString& friendId, const QString& body);

    void doMessageNormal(const gloox::Message& msg, QString& friendId);

    void joinRoom(gloox::MUCRoom* room);

    void cacheJoinRoom(const std::string& jid, const std::string& name = "");

    // Jingle session
    void handleSessionAction(gloox::Jingle::Action action, gloox::Jingle::Session* session,
                             const gloox::Jingle::Session::Jingle* jingle) override;

    void handleSessionActionError(gloox::Jingle::Action action, gloox::Jingle::Session* session,
                                  const gloox::Error* error) override;

    void handleIncomingSession(gloox::Jingle::Session* session) override;

private:
    CompatibleRecursiveMutex mutex;

    QStringList features;

    QString discoVal;
    QString _host;
    QString _username;
    QString _password;
    QString _resource;

    QString _nick;
    gloox::JID loginJid;
    std::unique_ptr<gloox::Client> _client;
    bool isConnected = false;

    // 发送消息的id
    std::set<std::string> sendIds;

    /**
     * k: sessionId
     * v: messageSession
     */
    std::map<std::string, gloox::MessageSession*> sessionMap;

    /**
     * k: bare
     * v: sessionId
     */
    std::map<std::string, std::string> sessionIdMap;

    /**
     * 在线
     * key: bare value:[resource，resource,...]
     */
    std::map<std::string, std::set<std::string>> onlineMap;

    std::map<IMPeerId, gloox::Jingle::RTP::Medias> mPeerRequestMedias;

    std::unique_ptr<gloox::VCardManager> vCardManager;
    std::unique_ptr<gloox::PubSub::Manager> pubSubManager;
    std::unique_ptr<gloox::BookmarkStorage> bookmarkStorage;
    std::unique_ptr<gloox::Registration> mRegistration;
    std::unique_ptr<gloox::NativeBookmarkStorage> m_nativeBookmark;

    std::unique_ptr<gloox::MessageEventFilter> m_messageEventFilter;

    QMap<QString, gloox::ChatStateFilter*> m_chatStateFilters;

    QMap<QString, IMRoomInfo> m_roomMap;



    QList<IMSessionHandler*> m_sessionHandlers;
    std::unique_ptr<gloox::Jingle::SessionManager> _sessionManager;

    //  ConferenceList mConferenceList;
    //  BookmarkList &mBookmarkList;

    // External Service Discovery
    std::vector<ortc::IceServer> mExtSrvDiscos;

    QMap<std::string, IMFromHostHandler*> fromHostHandlers;

    std::vector<SelfHandler*> selfHandlers;
    std::vector<FriendHandler*> friendHandlers;
    std::vector<GroupHandler*> groupHandlers;
    std::vector<IMHandler*> imHandlers;

signals:
    void exportEncryptedMessage(QString em);

    void receiveMessageReceipt(QString friendId, QString receipt);

    void incoming(QString xml);

    void doPubSubEventDone();

public slots:
    void sendServiceDiscoveryItems();
    void sendServiceDiscoveryInfo(const gloox::JID& item);
};

}  // namespace lib::messenger
