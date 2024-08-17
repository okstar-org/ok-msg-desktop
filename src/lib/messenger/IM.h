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

#include "base/r.h"
#include "base/system/sys_info.h"
#include "base/task.h"
#include "base/timer.h"

#include "IMGroup.h"
#include "lib/backend/OkCloudService.h"
#include "lib/messenger/IMFriend.h"
#include "lib/messenger/IMMessage.h"
#include "lib/session/AuthSession.h"
#include "messenger.h"

#include <memory>
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
};

namespace lib::ortc {
enum class JingleCallType;
}

namespace lib {
namespace messenger {


using namespace gloox;
using namespace gloox::PubSub;

struct IMRoomInfo {
    MUCRoom* room;

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

class IMJingle;

class IM : public ok::base::Task,
           public ConnectionListener,
#ifdef WANT_PING
           public PingHandler,
#endif
           public RegistrationHandler,
           public IncomingHandler,
           public ResultHandler,
           public RosterListener,
           public MUCRoomHandler,
           public MUCRoomConfigHandler,
           public MessageSessionHandler,
           public MessageHandler,
           public MessageEventHandler,
           public ChatStateHandler,
           public DiscoHandler,
           public DiscoNodeHandler,
           public PresenceHandler,
           public LogHandler,
           public VCardHandler,
           public TagHandler,
           public IqHandler,
           public BookmarkHandler,
           public NativeBookmarkHandler {
    Q_OBJECT
public:
    explicit IM(QString host, QString user, QString pwd, QStringList features);
    ~IM();

    inline static IMMessage fromXMsg(MsgType type, const gloox::Message& msg);

    std::unique_ptr<Client> makeClient();

    void setNickname(const QString& nickname);
    QString getNickname();

    void setAvatar(const QByteArray& avatar);
    void changePassword(const QString& password);
    IMContactId getSelfId();
    IMPeerId getSelfPeerId();
    QString getSelfUsername();

    /**
     * fetchVCard
     */
    void fetchFriendVCard(const QString& friendId);
    IMStatus getFriendStatus(const QString& friendId);
    void requestFriendNickname(const JID& friendId);

    /**
     * send
     */
    void sendPresence();
    void sendPresence(const JID& to, Presence::PresenceType type);
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
    RosterManager* enableRosterManager();

    void addFriend(const JID& jid, const QString& nick, const QString& msg);
    bool removeFriend(const JID& jid);

    void acceptFriendRequest(const QString&);
    void rejectFriendRequest(const QString&);

    size_t getRosterCount();
    void getRosterList(std::list<IMFriend>&);

    void setFriendAlias(const JID& jid, const std::string& alias);

    void retry();

    // gloox log
    void handleLog(LogLevel level, LogArea area, const std::string& message) override;

    virtual const JID& self() const { return _client->jid(); }

    Client* getClient() const { return _client.get(); }

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
    bool inviteToRoom(const JID& roomJid, const JID& peerId);
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

    void createRoom(const JID& jid, const std::string& password = "");

    const IMRoomInfo* findRoom(const QString& groupId) const;

    void doConnect();

    void doDisconnect();

    // 2-群组列表
    void loadGroupList();
    // 3-用户信息
    void loadRosterInfo();
    // 4-加入群聊
    void joinRooms();

    void send(const QString& xml);

    void stop();

    void interrupt();

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
                            Presence::PresenceType presenceType);

    [[nodiscard]] gloox::JID wrapJid(const QString& f) const;

    [[nodiscard]] gloox::JID wrapRoomJid(const QString& group) const;
    void sendChatState(const QString& to, ChatStateType state);

    void makeId(QString& id);

    //  ExtDisco &extDisco() { return mExtDisco; }

protected:
    void run() override;

    virtual void handleBookmarks(const BMConferenceList& cList) override;

#ifdef WANT_PING
    /**
     * ping handler
     */
    virtual void handlePing(const gloox::PingHandler::PingType type,
                            const std::string& body) override;
#endif

    /**
     * incoming handler
     */
    virtual void handleIncoming(Tag* tag) override;

    /**
     * iq handlers
     * @param iq
     * @return
     */
    bool handleIq(const IQ& iq) override;
    void handleIqID(const IQ& iq, int context) override;

    /**
     * tag handlers
     */
    void handleTag(Tag* tag) override;

    /**
     * connect handlers
     */
    void onConnect() override;
    void onDisconnect(ConnectionError e) override;
    bool onTLSConnect(const CertInfo& info) override;

    /**
     * Registration
     */
    virtual void handleRegistrationFields(const JID& from, int fields,
                                          std::string instructions) override;

    virtual void handleAlreadyRegistered(const JID& from) override;

    virtual void handleRegistrationResult(const JID& from,               //
                                          RegistrationResult regResult,  //
                                          const Error* error) override;

    virtual void handleDataForm(const JID& from, const DataForm& form) override;

    virtual void handleOOB(const JID& from, const OOB& oob) override;

    /**
     * vCard
     * @param jid
     * @param vch
     */
    void handleVCard(const JID& jid, const VCard* vcard) override;

    void handleVCardResult(VCardContext context, const JID& jid,
                           StanzaError error = StanzaErrorUndefined) override;

    /**
     * RosterListener
     * @param msg
     * @param session
     */
    void handleItemAdded(const gloox::JID&) override;
    void handleItemSubscribed(const JID& jid) override;
    void handleItemRemoved(const JID& jid) override;
    void handleItemUpdated(const JID& jid) override;
    void handleItemUnsubscribed(const JID& jid) override;
    void handleRoster(const Roster& roster) override;
    void handleRosterPresence(const RosterItem& item, const std::string& resource,
                              Presence::PresenceType presence, const std::string& msg) override;
    void handleSelfPresence(const RosterItem& item, const std::string& resource,
                            Presence::PresenceType presence, const std::string& msg) override;

    bool handleSubscriptionRequest(const JID& jid, const std::string& msg) override;

    bool handleUnsubscriptionRequest(const JID& jid, const std::string& msg) override;

    void handleNonrosterPresence(const Presence& presence) override;

    void handleRosterError(const IQ& iq) override;

    void handleRosterItemExchange(const JID& from, const RosterX* items) override;

    // MUC config
    void handleMUCConfigList(MUCRoom* room, const MUCListItemList& items,
                             MUCOperation operation) override;

    void handleMUCConfigForm(MUCRoom* room, const DataForm& form) override;

    void handleMUCConfigResult(MUCRoom* room, bool success, MUCOperation operation) override;

    void handleMUCRequest(MUCRoom* room, const DataForm& form) override;

    // MessageSessionHandler
    void handleMessage(const gloox::Message& msg, MessageSession* session = nullptr) override;
    void handleMessageSession(MessageSession* session) override;

    // MessageEventHandler
    void handleMessageEvent(const JID& from, const MessageEvent* event) override;

    void handleChatState(const JID& from, ChatStateType state) override;

    // MUC handler
    void handleMUCParticipantPresence(MUCRoom* room, const MUCRoomParticipant participant,
                                      const Presence& presence) override;

    void handleMUCMessage(MUCRoom* room, const gloox::Message& msg, bool priv) override;

    bool handleMUCRoomCreation(MUCRoom* room) override;

    void handleMUCSubject(MUCRoom* room, const std::string& nick,
                          const std::string& subject) override;

    void handleMUCInviteDecline(MUCRoom* room, const JID& invitee,
                                const std::string& reason) override;

    void handleMUCError(MUCRoom* room, StanzaError error) override;

    void handleMUCInfo(MUCRoom* room, int features, const std::string& name,
                       const DataForm* infoForm) override;

    void handleMUCItems(MUCRoom* room, const Disco::ItemList& items) override;

    // Disco handler
    void handleDiscoInfo(const JID& from, const Disco::Info&, int ontext) override;

    void handleDiscoItems(const JID& from, const Disco::Items&, int context) override;

    void handleDiscoError(const JID& from, const gloox::Error*, int context) override;
    // DiscoNodeHandler
    virtual StringList handleDiscoNodeFeatures(const JID& from, const std::string& node) override;

    virtual Disco::IdentityList handleDiscoNodeIdentities(const JID& from,
                                                          const std::string& node) override;

    virtual Disco::ItemList handleDiscoNodeItems(const JID& from, const JID& to,
                                                 const std::string& node = EmptyString) override;

    // Presence handler
    void handlePresence(const Presence& presence) override;

    /**
     * Receives the payload for an item.
     *
     * @param service Service hosting the queried node.
     * @param node ID of the parent node.
     * @param entry The complete item Tag (do not delete).
     */
    void handleItem(const JID& service, const std::string& node, const Tag* entry) override;

    /**
     * Receives the list of Items for a node.
     *
     * @param id The reply IQ's id.
     * @param service Service hosting the queried node.
     * @param node ID of the queried node (empty for the root node).
     * @param itemList List of contained items.
     * @param error Describes the error case if the request failed.
     *
     * @see Manager::requestItems()
     */
    void handleItems(const std::string& id, const JID& service, const std::string& node,
                     const ItemList& itemList, const gloox::Error* error = 0) override;

    /**
     * Receives the result for an item publication.
     *
     * @param id The reply IQ's id.
     * @param service Service hosting the queried node.
     * @param node ID of the queried node. If empty, the root node has been
     * queried.
     * @param itemList List of contained items.
     * @param error Describes the error case if the request failed.
     *
     * @see Manager::publishItem
     */
    void handleItemPublication(const std::string& id, const JID& service, const std::string& node,
                               const ItemList& itemList, const gloox::Error* error = 0) override;

    /**
     * Receives the result of an item removal.
     *
     * @param id The reply IQ's id.
     * @param service Service hosting the queried node.
     * @param node ID of the queried node. If empty, the root node has been
     * queried.
     * @param itemList List of contained items.
     * @param error Describes the error case if the request failed.
     *
     * @see Manager::deleteItem
     */
    void handleItemDeletion(const std::string& id, const JID& service, const std::string& node,
                            const ItemList& itemList, const gloox::Error* error = 0) override;

    /**
     * Receives the subscription results. In case a problem occured, the
     * Subscription ID and SubscriptionType becomes irrelevant.
     *
     * @param id The reply IQ's id.
     * @param service PubSub service asked for subscription.
     * @param node Node asked for subscription.
     * @param sid Subscription ID.
     * @param jid Subscribed entity.
     * @param subType Type of the subscription.
     * @param error Subscription Error.
     *
     * @see Manager::subscribe
     */
    void handleSubscriptionResult(const std::string& id, const JID& service,
                                  const std::string& node, const std::string& sid, const JID& jid,
                                  const gloox::PubSub::SubscriptionType subType,
                                  const gloox::Error* error = 0) override;

    /**
     * Receives the unsubscription results. In case a problem occured, the
     * subscription ID becomes irrelevant.
     *
     * @param id The reply IQ's id.
     * @param service PubSub service.
     * @param error Unsubscription Error.
     *
     * @see Manager::unsubscribe
     */
    void handleUnsubscriptionResult(const std::string& id, const JID& service,
                                    const gloox::Error* error = 0) override;

    /**
     * Receives the subscription options for a node.
     *
     * @param id The reply IQ's id.
     * @param service Service hosting the queried node.
     * @param jid Subscribed entity.
     * @param node ID of the node.
     * @param options Options DataForm.
     * @param sid An optional subscription ID.
     * @param error Subscription options retrieval Error.
     *
     * @see Manager::getSubscriptionOptions
     */
    void handleSubscriptionOptions(const std::string& id, const JID& service, const JID& jid,
                                   const std::string& node, const DataForm* options,
                                   const std::string& sid = EmptyString,
                                   const gloox::Error* error = 0) override;

    /**
     * Receives the result for a subscription options modification.
     *
     * @param id The reply IQ's id.
     * @param service Service hosting the queried node.
     * @param jid Subscribed entity.
     * @param node ID of the queried node.
     * @param sid An optional subscription ID.
     * @param error Subscription options modification Error.
     *
     * @see Manager::setSubscriptionOptions
     */
    void handleSubscriptionOptionsResult(const std::string& id, const JID& service, const JID& jid,
                                         const std::string& node,
                                         const std::string& sid = EmptyString,
                                         const gloox::Error* error = 0) override;

    /**
     * Receives the list of subscribers to a node.
     *
     * @param id The reply IQ's id.
     * @param service Service hosting the node.
     * @param node ID of the queried node.
     * @param list Subscriber list.
     * @param error Subscription options modification Error.
     *
     * @see Manager::getSubscribers
     */
    void handleSubscribers(const std::string& id, const JID& service, const std::string& node,
                           const SubscriptionList& list, const gloox::Error* error = 0) override;

    /**
     * Receives the result of a subscriber list modification.
     *
     * @param id The reply IQ's id.
     * @param service Service hosting the node.
     * @param node ID of the queried node.
     * @param list Subscriber list.
     * @param error Subscriber list modification Error.
     *
     * @see Manager::setSubscribers
     */
    void handleSubscribersResult(const std::string& id, const JID& service, const std::string& node,
                                 const SubscriberList* list,
                                 const gloox::Error* error = 0) override;

    /**
     * Receives the affiliate list for a node.
     *
     * @param id The reply IQ's id.
     * @param service Service hosting the node.
     * @param node ID of the queried node.
     * @param list Affiliation list.
     * @param error Affiliation list retrieval Error.
     *
     * @see Manager::getAffiliates
     */
    void handleAffiliates(const std::string& id, const JID& service, const std::string& node,
                          const AffiliateList* list, const gloox::Error* error = 0) override;

    /**
     * Handle the affiliate list for a specific node.
     *
     * @param id The reply IQ's id.
     * @param service Service hosting the node.
     * @param node ID of the node.
     * @param list The Affiliate list.
     * @param error Affiliation list modification Error.
     *
     * @see Manager::setAffiliations
     */
    void handleAffiliatesResult(const std::string& id, const JID& service, const std::string& node,
                                const AffiliateList* list, const gloox::Error* error = 0) override;

    /**
     * Receives the configuration for a specific node.
     *
     * @param id The reply IQ's id.
     * @param service Service hosting the node.
     * @param node ID of the node.
     * @param config Configuration DataForm.
     * @param error Configuration retrieval Error.
     *
     * @see Manager::getNodeConfig
     */
    void handleNodeConfig(const std::string& id, const JID& service, const std::string& node,
                          const DataForm* config, const gloox::Error* error = 0) override;

    /**
     * Receives the result of a node's configuration modification.
     *
     * @param id The reply IQ's id.
     * @param service Service hosting the node.
     * @param node ID of the node.
     * @param error Configuration modification Error.
     *
     * @see Manager::setNodeConfig
     */
    void handleNodeConfigResult(const std::string& id, const JID& service, const std::string& node,
                                const gloox::Error* error = 0) override;

    /**
     * Receives the result of a node creation.
     *
     * @param id The reply IQ's id.
     * @param service Service hosting the node.
     * @param node ID of the node.
     * @param error Node creation Error.
     *
     * @see Manager::setNodeConfig
     */
    void handleNodeCreation(const std::string& id, const JID& service, const std::string& node,
                            const gloox::Error* error = 0) override;

    /**
     * Receives the result for a node removal.
     *
     * @param id The reply IQ's id.
     * @param service Service hosting the node.
     * @param node ID of the node.
     * @param error Node removal Error.
     *
     * @see Manager::deleteNode
     */
    void handleNodeDeletion(const std::string& id, const JID& service, const std::string& node,
                            const gloox::Error* error = 0) override;

    /**
     * Receives the result of a node purge request.
     *
     * @param id The reply IQ's id.
     * @param service Service hosting the node.
     * @param node ID of the node.
     * @param error Node purge Error.
     *
     * @see Manager::purgeNode
     */
    void handleNodePurge(const std::string& id, const JID& service, const std::string& node,
                         const gloox::Error* error = 0) override;

    /**
     * Receives the Subscription list for a specific service.
     *
     * @param id The reply IQ's id.
     * @param service The queried service.
     * @param subMap The map of node's subscription.
     * @param error Subscription list retrieval Error.
     *
     * @see Manager::getSubscriptions
     */
    void handleSubscriptions(const std::string& id, const JID& service,
                             const SubscriptionMap& subMap, const gloox::Error* error = 0) override;

    /**
     * Receives the Affiliation map for a specific service.
     *
     * @param id The reply IQ's id.
     * @param service The queried service.
     * @param affMap The map of node's affiliation.
     * @param error Affiliation list retrieval Error.
     *
     * @see Manager::getAffiliations
     */
    void handleAffiliations(const std::string& id, const JID& service, const AffiliationMap& affMap,
                            const gloox::Error* error = 0) override;

    /**
     * Receives the default configuration for a specific node type.
     *
     * @param id The reply IQ's id.
     * @param service The queried service.
     * @param config Configuration form for the node type.
     * @param error Default node config retrieval Error.
     *
     * @see Manager::getDefaultNodeConfig
     */
    void handleDefaultNodeConfig(const std::string& id,   //
                                 const JID& service,      //
                                 const DataForm* config,  //
                                 const gloox::Error* error = 0) override;

    void handleBookmarks(const BookmarkList& bList,  //
                         const ConferenceList& cList) override;

    void doPubSubEvent(const gloox::PubSub::Event* pse, const gloox::Message& msg, QString& friendId);
    void doMessageHeadline(const gloox::Message& msg, QString& friendId, const QString& body);
    void doMessageChat(const gloox::Message& msg, QString& friendId, const QString& body);

    void doMessageNormal(const gloox::Message& msg, QString& friendId);

    void joinRoom(MUCRoom* room);

    void cacheJoinRoom(const std::string& jid, const std::string& name = "");

private:
    QMutex m_mutex;

    QStringList features;

    QString discoVal;
    QString _host;
    QString _username;
    QString _password;
    QString _resource;

    QString _nick;

    std::unique_ptr<Client> _client;

    // 发送消息的id
    std::set<std::string> sendIds;

    /**
     * k: sessionId
     * v: messageSession
     */
    std::map<std::string, MessageSession*> sessionMap;

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

    std::map<IMPeerId, Jingle::RTP::Medias> mPeerRequestMedias;

    std::unique_ptr<VCardManager> vCardManager;
    std::unique_ptr<PubSub::Manager> pubSubManager;
    std::unique_ptr<BookmarkStorage> bookmarkStorage;
    std::unique_ptr<Registration> mRegistration;
    std::unique_ptr<NativeBookmarkStorage> m_nativeBookmark;

    std::unique_ptr<MessageEventFilter> m_messageEventFilter;

    QMap<QString, ChatStateFilter*> m_chatStateFilters;

    QMap<QString, IMRoomInfo> m_roomMap;

    QThread* thread;

    //
    //  ConferenceList mConferenceList;
    //  BookmarkList &mBookmarkList;
    //  ExtDisco mExtDisco;

signals:
    void connectResult(IMConnectStatus);

    void receiveRoomMessage(QString groupId, IMPeerId friendId, IMMessage);

    // friend events
    void receiveFriend(IMFriend frnd);

    void receiveFriendRequest(QString friendId, QString msg);

    void receiveFriendRemoved(QString friendId);

    void receiveFriendStatus(QString friendId, int status);

    void receiveMessageSession(QString contactId, QString sid);

    void receiveFriendMessage(QString peerId, IMMessage);

    void receiveNicknameChange(QString friendId, QString nickname);

    void receiveFriendAliasChanged(JID friendId, std::string alias);

    void receiveFriendAvatarChanged(QString friendId, std::string avatar);

    void receiveFriendChatState(QString friendId, int state);

    void exportEncryptedMessage(QString em);

    void receiveMessageReceipt(QString friendId, QString receipt);

    void incoming(QString xml);

    // Self events
    void selfIdChanged(QString id);
    void selfNicknameChanged(QString nickname);
    void selfAvatarChanged(std::string avatar);
    void selfStatusChanged(int type, const std::string status);

    void started();
    void stopped();

    void groupReceived(const QString groupId, const QString name);
    void groupListReceivedDone();
    void groupOccupants(const QString groupId, const uint size);
    void groupOccupantStatus(const QString& groupId, IMGroupOccupant occ);
    void groupInvite(const QString groupId, const QString peerId, const QString message);

    void groupRoomInfo(QString groupId, IMGroup groupInfo);

    void groupSubjectChanged(const JID group, const std::string subject);

    void doPubSubEventDone();

public slots:
    void sendServiceDiscoveryItems();
    void sendServiceDiscoveryInfo(const JID& item);
};

}  // namespace messenger
}  // namespace lib
