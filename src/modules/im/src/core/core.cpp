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

#include "core.h"
#include "corefile.h"

#include "lib/messenger/messenger.h"
#include "lib/session/AuthSession.h"

#include "src/core/coreav.h"
#include "src/core/dhtserver.h"
#include "src/core/icoresettings.h"
#include "src/core/toxlogger.h"
#include "src/core/toxoptions.h"
#include "src/core/toxstring.h"
#include "src/model/groupinvite.h"
#include "src/model/status.h"
// #include "src/net/bootstrapnodeupdater.h"
#include "base/compatiblerecursivemutex.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/util/strongtype.h"

#include <QCoreApplication>
#include <vector>
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
#include <QRandomGenerator>
#endif

#include <QString>
#include <QStringBuilder>
#include <QTimer>

#include <cassert>
#include <chrono>
#include <thread>

const QString Core::TOX_EXT = ".tox";

#define ASSERT_CORE_THREAD assert(QThread::currentThread() == coreThread.get())

namespace {
bool LogConferenceTitleError(TOX_ERR_CONFERENCE_TITLE error) {
  switch (error) {
  case TOX_ERR_CONFERENCE_TITLE_OK:
    break;
  case TOX_ERR_CONFERENCE_TITLE_CONFERENCE_NOT_FOUND:
    qWarning() << "Conference title not found";
    break;
  case TOX_ERR_CONFERENCE_TITLE_INVALID_LENGTH:
    qWarning() << "Invalid conference title length";
    break;
  case TOX_ERR_CONFERENCE_TITLE_FAIL_SEND:
    qWarning() << "Failed to send title packet";
  }
  return error;
}

bool parseToxErrBootstrap(Tox_Err_Bootstrap error) {
  switch (error) {
  case TOX_ERR_BOOTSTRAP_OK:
    return true;

  case TOX_ERR_BOOTSTRAP_NULL:
    qCritical() << "null argument when not expected";
    return false;

  case TOX_ERR_BOOTSTRAP_BAD_HOST:
    qCritical() << "Could not resolve hostname, or invalid IP address";
    return false;

  case TOX_ERR_BOOTSTRAP_BAD_PORT:
    qCritical() << "out of range port";
    return false;

  default:
    qCritical() << "Unknown Tox_Err_bootstrap error code:" << error;
    return false;
  }
}

bool parseFriendSendMessageError(Tox_Err_Friend_Send_Message error) {
  switch (error) {
  case TOX_ERR_FRIEND_SEND_MESSAGE_OK:
    return true;
  case TOX_ERR_FRIEND_SEND_MESSAGE_NULL:
    qCritical() << "Send friend message passed an unexpected null argument";
    return false;
  case TOX_ERR_FRIEND_SEND_MESSAGE_FRIEND_NOT_FOUND:
    qCritical() << "Send friend message could not find friend";
    return false;
  case TOX_ERR_FRIEND_SEND_MESSAGE_FRIEND_NOT_CONNECTED:
    qCritical() << "Send friend message: friend is offline";
    return false;
  case TOX_ERR_FRIEND_SEND_MESSAGE_SENDQ:
    qCritical() << "Failed to allocate more message queue";
    return false;
  case TOX_ERR_FRIEND_SEND_MESSAGE_TOO_LONG:
    qCritical() << "Attemped to send message that's too long";
    return false;
  case TOX_ERR_FRIEND_SEND_MESSAGE_EMPTY:
    qCritical() << "Attempted to send an empty message";
    return false;
  default:
    qCritical() << "Unknown friend send message error:"
                << static_cast<int>(error);
    return false;
  }
}

bool parseConferenceSendMessageError(Tox_Err_Conference_Send_Message error) {
  switch (error) {
  case TOX_ERR_CONFERENCE_SEND_MESSAGE_OK:
    return true;

  case TOX_ERR_CONFERENCE_SEND_MESSAGE_CONFERENCE_NOT_FOUND:
    qCritical() << "Conference not found";
    return false;

  case TOX_ERR_CONFERENCE_SEND_MESSAGE_FAIL_SEND:
    qCritical() << "Conference message failed to send";
    return false;

  case TOX_ERR_CONFERENCE_SEND_MESSAGE_NO_CONNECTION:
    qCritical() << "No connection";
    return false;

  case TOX_ERR_CONFERENCE_SEND_MESSAGE_TOO_LONG:
    qCritical() << "Message too long";
    return false;
  default:
    qCritical() << "Unknown Tox_Err_Conference_Send_Message  error:"
                << static_cast<int>(error);
    return false;
  }
}
} // namespace

Core::Core(QThread *coreThread)
    : tox(nullptr), av(nullptr), toxTimer{new QTimer{this}},
      coreThread(coreThread) {

  assert(toxTimer);

  toxTimer->setSingleShot(true);
  connect(toxTimer, &QTimer::timeout, this, &Core::process);
  connect(coreThread, &QThread::finished, toxTimer, &QTimer::stop);

  //  tox = lib::IM::Messenger::getInstance() ;
}

Core::~Core() {
  /*
   * First stop the thread to stop the timer and avoid Core emitting callbacks
   * into an already destructed CoreAV.
   */
  coreThread->exit(0);
  coreThread->wait();

  av.reset();
  tox.reset();
}

Status::Status Core::fromToxStatus(const Tox_User_Status &userStatus) const {
  Status::Status status;
  switch (userStatus) {
  case TOX_USER_STATUS_Available:
  case TOX_USER_STATUS_Chat:
    status = Status::Status::Online;
    break;
  case TOX_USER_STATUS_Away:
    status = Status::Status::Away;
    break;
  case TOX_USER_STATUS_DND:
    status = Status::Status::Busy;
    break;
  default:
    status = Status::Status::Offline;
    break;
  }
  return status;
}

/**
 * @brief Registers all toxcore callbacks
 * @param tox Tox instance to register the callbacks on
 */
void Core::registerCallbacks(Tox *tox) {
  tox->addFriendHandler(this);
  tox->addGroupHandler(this);
  tox->addSelfHandler(this);

  connect(tox, &lib::messenger::Messenger::connected, [this](){
    emit connected();
  });
}

/**
 * @brief Factory method for the Core object
 * @param savedata empty if new profile or saved data else
 * @param settings Settings specific to Core
 * @return nullptr or a Core object ready to start
 */
ToxCorePtr Core::makeToxCore(const QByteArray &savedata,
                             const ICoreSettings *const settings,
                             ToxCoreErrors *err) {

  QThread *thread = new QThread();
  if (!thread) {
    qCritical() << "could not allocate Core thread";
    return {};
  }
  thread->setObjectName("Core");

  auto toxOptions = ToxOptions::makeToxOptions(savedata, settings);
  if (toxOptions == nullptr) {
    qCritical() << "could not allocate Tox Options data structure";
    if (err) {
      *err = ToxCoreErrors::ERROR_ALLOC;
    }
    return {};
  }

  ToxCorePtr core(new Core(thread));
  if (core == nullptr) {
    if (err) {
      *err = ToxCoreErrors::ERROR_ALLOC;
    }
    return {};
  }

  //  Tox_Err_New tox_err;
  //  core->tox = ToxPtr(tox_new(*toxOptions, &tox_err));
  core->tox = ToxPtr(lib::messenger::Messenger::getInstance());
  //  switch (tox_err) {
  //  case TOX_ERR_NEW_OK:
  //    break;
  //
  //  case TOX_ERR_NEW_LOAD_BAD_FORMAT:
  //    qCritical() << "failed to parse Tox save data";
  //    if (err) {
  //      *err = ToxCoreErrors::BAD_PROXY;
  //    }
  //    return {};
  //
  //  case TOX_ERR_NEW_PORT_ALLOC:
  //    if (toxOptions->getIPv6Enabled()) {
  //      toxOptions->setIPv6Enabled(false);
  //      core->tox = ToxPtr(tox_new(*toxOptions, &tox_err));
  //      if (tox_err == TOX_ERR_NEW_OK) {
  //        qWarning() << "Core failed to start with IPv6, falling back to IPv4.
  //        "
  //                      "LAN discovery "
  //                      "may not work properly.";
  //        break;
  //      }
  //    }
  //
  //    qCritical() << "can't to bind the port";
  //    if (err) {
  //      *err = ToxCoreErrors::FAILED_TO_START;
  //    }
  //    return {};
  //
  //  case TOX_ERR_NEW_PROXY_BAD_HOST:
  //  case TOX_ERR_NEW_PROXY_BAD_PORT:
  //  case TOX_ERR_NEW_PROXY_BAD_TYPE:
  //    qCritical() << "bad proxy, error code:" << tox_err;
  //    if (err) {
  //      *err = ToxCoreErrors::BAD_PROXY;
  //    }
  //    return {};
  //
  //  case TOX_ERR_NEW_PROXY_NOT_FOUND:
  //    qCritical() << "proxy not found";
  //    if (err) {
  //      *err = ToxCoreErrors::BAD_PROXY;
  //    }
  //    return {};
  //
  //  case TOX_ERR_NEW_LOAD_ENCRYPTED:
  //    qCritical() << "attempted to load encrypted Tox save data";
  //    if (err) {
  //      *err = ToxCoreErrors::INVALID_SAVE;
  //    }
  //    return {};
  //
  //  case TOX_ERR_NEW_MALLOC:
  //    qCritical() << "memory allocation failed";
  //    if (err) {
  //      *err = ToxCoreErrors::ERROR_ALLOC;
  //    }
  //    return {};
  //
  //  case TOX_ERR_NEW_NULL:
  //    qCritical() << "a parameter was null";
  //    if (err) {
  //      *err = ToxCoreErrors::FAILED_TO_START;
  //    }
  //    return {};
  //
  //  default:
  //    qCritical() << "Tox core failed to start, unknown error code:" <<
  //    tox_err; if (err) {
  //      *err = ToxCoreErrors::FAILED_TO_START;
  //    }
  //    return {};
  //  }

  // tox should be valid by now
  assert(core->tox != nullptr);

  // toxcore is successfully created, create toxav
  // TODO(sudden6): don't create CoreAv here, Core should be usable without
  // CoreAV
  core->av = CoreAV::makeCoreAV(core->tox.get(), core->coreLoopLock);
  if (!core->av) {
    qCritical() << "Toxav failed to start";
    if (err) {
      *err = ToxCoreErrors::FAILED_TO_START;
    }
    return {};
  }

  // create CoreFile
  core->file =
      CoreFile::makeCoreFile(core.get(), core->tox.get(), core->coreLoopLock);
  if (!core->file) {
    qCritical() << "CoreFile failed to start";
    if (err) {
      *err = ToxCoreErrors::FAILED_TO_START;
    }
    return {};
  }

  core->registerCallbacks(core->tox.get());

  // connect the thread with the Core
  connect(thread, &QThread::started, core.get(), &Core::onStarted);
  core->moveToThread(thread);

  // when leaving this function 'core' should be ready for it's start() action
  // or a nullptr
  return core;
}

void Core::onStarted() {
  qDebug() << "connected...";
  ASSERT_CORE_THREAD;

  // One time initialization stuff
  QString name = getUsername();
  qDebug() << "username:" << name;
  if (!name.isEmpty()) {
    emit usernameSet(name);
  }

  auto status = getStatus();
  emit statusSet(status);

  QString msg = getStatusMessage();
  if (!msg.isEmpty()) {
    emit statusMessageSet(msg);
  }

  //  ToxId id = getSelfId();
  //  // Id comes from toxcore, must be valid
  //  assert(id.isValid());
  //  emit idSet(id);
  //  std::this_thread::sleep_for(std::chrono::seconds(4)); // sleep 5秒

  // loadFriends();
  // loadGroups();
  tox->start();
  av->start();
  process(); // starts its own timer
  emit avReady();
  qDebug() << "connected completed.";
}

/**
 * @brief Starts toxcore and it's event loop, can be called from any thread
 */
void Core::start() {
  qDebug() << __func__ << "...";
  coreThread->start();
}

/**
 * @brief Returns the global widget's Core instance
 */
Core *Core::getInstance() { return Nexus::getCore(); }

const CoreAV *Core::getAv() const { return av.get(); }

CoreAV *Core::getAv() { return av.get(); }

CoreFile *Core::getCoreFile() const { return file.get(); }

/* Using the now commented out statements in checkConnection(), I watched how
 * many ticks disconnects-after-initial-connect lasted. Out of roughly 15
 * trials, 5 disconnected; 4 were DCd for less than 20 ticks, while the 5th was
 * ~50 ticks. So I set the tolerance here at 25, and initial DCs should be very
 * rare now. This should be able to go to 50 or 100 without affecting legitimate
 * disconnects' downtime, but lets be conservative for now. Edit: now ~~40~~ 30.
 */
#define CORE_DISCONNECT_TOLERANCE 30

/**
 * @brief Processes toxcore events and ensure we stay connected, called by its
 * own timer
 */
void Core::process() {
  QMutexLocker ml{&coreLoopLock};

  ASSERT_CORE_THREAD;

  static int tolerance = CORE_DISCONNECT_TOLERANCE;
  //  tox_iterate(tox.get(), this);

#ifdef DEBUG
  // we want to see the debug messages immediately
  fflush(stdout);
#endif

  // TODO(sudden6): recheck if this is still necessary
  //  if (checkConnection()) {
  //    tolerance = CORE_DISCONNECT_TOLERANCE;
  //  } else if (!(--tolerance)) {
  //    bootstrapDht();
  //    tolerance = 3 * CORE_DISCONNECT_TOLERANCE;
  //  }

  //  unsigned sleeptime = qMin(tox_iteration_interval(tox.get()),
  //                            getCoreFile()->corefileIterationInterval());
  //  toxTimer->start(sleeptime);
}

bool Core::checkConnection() {
  ASSERT_CORE_THREAD;
  static bool isConnected = false;
  //  bool toxConnected =tox_self_get_connection_status(tox.get()) !=
  //  TOX_CONNECTION_NONE; if (toxConnected && !isConnected) {
  //    qDebug() << "Connected to the DHT";
  //    emit connected();
  //  } else if (!toxConnected && isConnected) {
  //    qDebug() << "Disconnected from the DHT";
  //    emit disconnected();
  //  }
  //
  //  isConnected = toxConnected;
  //  return toxConnected;
  return true;
}

/**
 * @brief Connects us to the Tox network
 */
void Core::bootstrapDht() {
  //  ASSERT_CORE_THREAD;

  //  QList<DhtServer> bootstrapNodes =
  //      BootstrapNodeUpdater::loadDefaultBootstrapNodes();
  //
  //  int listSize = bootstrapNodes.size();
  //  if (!listSize) {
  //    qWarning() << "no bootstrap list?!?";
  //    return;
  //  }

  //  int i = 0;
  // #if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
  //  static int j = QRandomGenerator::global()->generate() % listSize;
  // #else
  //  static int j = qrand() % listSize;
  // #endif
  // i think the more we bootstrap, the more we jitter because the more we
  // overwrite nodes
  //  while (i < 2) {
  //    const DhtServer &dhtServer = bootstrapNodes[j % listSize];
  //    QString dhtServerAddress = dhtServer.address.toLatin1();
  //    QString port = QString::number(dhtServer.port);
  //    QString name = dhtServer.name;
  //    qDebug() << QString("Connecting to a bootstrap node...");
  //    QByteArray address = dhtServer.address.toLatin1();
  //    // TODO: constucting the pk via ToxId is a workaround
  //    ToxPk pk = ToxId{dhtServer.userId}.getPublicKey();

  //    const uint8_t *pkPtr = pk.getData();
  //
  //    Tox_Err_Bootstrap error;
  //    tox_bootstrap(tox.get(), address.constData(), dhtServer.port, pkPtr,
  //                  &error);
  //    parseToxErrBootstrap(error);
  //
  //    tox_add_tcp_relay(tox.get(), address.constData(), dhtServer.port,
  //    pkPtr,
  //                      &error);
  //    parseToxErrBootstrap(error);

  //    ++j;
  //    ++i;
  //  }
}

void Core::onFriend(const QString friendId) {
  qDebug() << __func__ << friendId;
  emit sig_friendAdded(  ToxPk(friendId), true);
}

void Core::onFriendDone() {
  qDebug() << "onFriend done.";
  emit friendAddedDone();
}


void Core::onFriendRequest(const QString friendId,
                           QString msg) {
  qDebug() << "onFriendRequest" << friendId;
  emit friendRequestReceived(ToxPk(friendId), msg);
}

void Core::onFriendRemoved(QString friendId) {
  qDebug() << "removed" << friendId;
  emit friendRemoved(friendId);
}

void Core::onFriendStatus(QString friendId, Tox_User_Status status) {
  qDebug() << "onFriendStatus friendId:" << friendId << status;
  onUserStatusChanged(nullptr, friendId, status, this);
}

void Core::onFriendRequest(Tox *, const QString &cFriendPk,
                           const uint8_t *cMessage, size_t cMessageSize,
                           void *core) {
  //  ToxPk friendPk(cFriendPk);
  //  QString requestMessage = ToxString(cMessage, cMessageSize).getQString();
  //  emit static_cast<Core *>(core)->friendRequestReceived(friendPk,
  //                                                        requestMessage);
}

void Core::onFriendMessage(QString friendId,
                           lib::messenger::IMMessage message) {
  qDebug() << "msg content:" << message.body;
  qDebug() << "from friend:" << friendId;
  sendReceiptReceived(friendId, message.id);
  lib::messenger::FriendId ff(friendId);
  emit friendMessageReceived(ToxPk(ff), message, false);
}

/**
 * 实现ChatState
 */
void Core::onFriendChatState(QString friendId, int state) {
  qDebug() << "onFriendChatState:" << friendId << "state:" << state;
  emit friendTypingChanged(getFriendPublicKey(friendId), state == 2);
}

void Core::onFriendNameChanged(QString friendId, QString name) {
  qDebug() << "onFriendNameChanged" << friendId << "alias:" << name;
  emit friendUsernameChanged(getFriendPublicKey(friendId), name);
}

void Core::onFriendAvatarChanged(const QString friendId,
                                 const std::string avatar) {
  if (avatar.empty())
    return;
  qDebug() << "onFriendAvatarChanged" << friendId <<"avatar size"<< avatar.size();
  emit friendAvatarChanged(getFriendPublicKey(friendId), QByteArray::fromStdString(avatar));
}

void Core::onUserStatusChanged(Tox *, QString friendId,
                               Tox_User_Status userStatus, void *core) {
  qDebug() << "onUserStatusChanged:" << friendId << "userStatus:" << userStatus;
  Status::Status status = fromToxStatus(userStatus);
  emit friendStatusChanged(getFriendPublicKey(friendId), status);
}

void Core::onGroupList(const QString groupId, const QString name) {
  qDebug() << __func__ << "groupId:" << groupId << name;
  emit groupJoined( GroupId (groupId), name);
}

void Core::onGroupListDone() {
  qDebug() << "onGroupListDone";
  emit groupJoinedDone();
}

void Core::onGroupInvite(Tox *tox, QString friendId, Tox_Conference_Type type,
                         const uint8_t *cookie, size_t length, void *vCore) {
  Core *core = static_cast<Core *>(vCore);
  const QByteArray data(reinterpret_cast<const char *>(cookie), length);
  const GroupInvite inviteInfo("", friendId, type, data);
  switch (type) {
  case TOX_CONFERENCE_TYPE_TEXT:
    qDebug() << QString("Text group invite by %1").arg(friendId);
    emit core->groupInviteReceived(inviteInfo);
    break;

  case TOX_CONFERENCE_TYPE_AV:
    qDebug() << QString("AV group invite by %1").arg(friendId);
    emit core->groupInviteReceived(inviteInfo);
    break;

  default:
    qWarning() << "Group invite with unknown type " << type;
  }
}

void Core::onGroupPeerListChange(Tox *, QString groupId, void *vCore) {
  const auto core = static_cast<Core *>(vCore);
  qDebug() << QString("Group %1 peerlist changed").arg(groupId);
  // no saveRequest, this callback is called on every connection to group peer,
  // not just on brand new peers
  emit core->groupPeerlistChanged(groupId);
}

void Core::onGroupPeerNameChange(Tox *, QString groupId, QString peerId,
                                 const uint8_t *name, size_t length,
                                 void *vCore) {
  const auto newName = ToxString(name, length).getQString();
  qDebug() << QString("Group %1, Peer %2, name changed to %3")
                  .arg(groupId)
                  .arg(peerId)
                  .arg(newName);
  auto *core = static_cast<Core *>(vCore);
  auto peerPk = core->getGroupPeerPk(groupId, peerId);
  emit core->groupPeerNameChanged(groupId, peerPk, newName);
}

void Core::onGroupTitleChange(Tox *, QString groupId, QString peerId,
                              const uint8_t *cTitle, size_t length,
                              void *vCore) {
  Core *core = static_cast<Core *>(vCore);
  QString author = core->getGroupPeerName(groupId, peerId);
  emit core->saveRequest();
  emit core->groupTitleChanged(groupId, author,
                               ToxString(cTitle, length).getQString());
}

void Core::onGroupInvite(const QString groupId, const QString peerId,
                         const QString message) {

  qDebug() << "onGroupInvite groupId:" << groupId << " friendId:" << peerId
           << " msg:" << message;

  GroupInvite invite(groupId, peerId, TOX_CONFERENCE_TYPE_TEXT,
                     message.toUtf8());
  emit groupInviteReceived(invite);
}

void Core::onGroupMessage(const QString groupId,
                          const lib::messenger::PeerId peerId,
                          const lib::messenger::IMMessage message) {

  //qDebug() << "onGroupMessage groupId:" << groupId //
  //         << "peerId:" << peerId.toString()       //
  //         << "from:" << message.from              //
  //         << "msg:" << message.body;
  bool isAction = false;
  emit groupMessageReceived(groupId, peerId.resource, message.from,
                            message.body, message.time, isAction);
}

void Core::onGroupOccupants(const QString groupId, const uint size) {
  emit groupPeerSizeChanged(groupId, size);
}

void Core::onGroupOccupantStatus(const QString groupId, //
                                 const QString peerId,  //
                                 bool online) {
  emit groupPeerStatusChanged(groupId, peerId, online);
}

void Core::onGroupInfo( QString groupId, lib::messenger::GroupInfo groupInfo) {
  emit groupTitleChanged(groupId, QString(), qstring(groupInfo.name));
}

void Core::onMessageReceipt(QString friendId, ReceiptNum receipt) {
  emit receiptRecieved(getFriendPublicKey(friendId), receipt);
}

void Core::acceptFriendRequest(const ToxPk &friendPk) {
  QMutexLocker ml{&coreLoopLock};
  //   TODO: error handling
  QString friendId = friendPk.toString(); // tox_friend_add_norequest(tox.get(),
                                          // friendPk.getData(), nullptr);
  qDebug() << friendId;
  tox->acceptFriendRequest(friendId);

  //    if (friendId == std::numeric_limits<uint32_t>::max()) {
  //      emit failedToAddFriend(friendPk);
  //    } else {
  emit saveRequest();
  emit sig_friendAdded(friendPk, true);
  //    }
}

void Core::rejectFriendRequest(const ToxPk &friendPk) {
  tox->rejectFriendRequest(friendPk.toString());
}

/**
 * @brief Checks that sending friendship request is correct and returns error
 * message accordingly
 * @param friendId Id of a friend which request is destined to
 * @param message Friendship request message
 * @return Returns empty string if sending request is correct, according error
 * message otherwise
 */
QString Core::getFriendRequestErrorMessage(const ToxId &friendId,
                                           const QString &message) const {
  QMutexLocker ml{&coreLoopLock};

  if (!friendId.isValid()) {
    return tr("Invalid Ok ID", "Error while sending friendship request");
  }

  if (message.isEmpty()) {
    return tr("You need to write a message with your request",
              "Error while sending friendship request");
  }

  //  if (message.length() > static_cast<int>(tox_max_friend_request_length()))
  //  {
  //    return tr("Your message is too long!",
  //              "Error while sending friendship request");
  //  }

  //  if (hasFriendWithPublicKey(friendId.getPublicKey())) {
  return tr("Friend is already added",
            "Error while sending friendship request");
  //  }

  return QString{};
}

void Core::requestFriendship(const ToxId &friendId,const QString &nick, const QString &message) {
  QMutexLocker ml{&coreLoopLock};

  // ToxPk friendPk = friendId.getPublicKey();
  // QString errorMessage = getFriendRequestErrorMessage(friendId, message);
  // if (!errorMessage.isNull()) {
  //   emit failedToAddFriend(friendPk, errorMessage);
  //   emit saveRequest();
  //   return;
  // }

  //  ToxString cMessage(message);
  QString fId = friendId.getToxIdAsStr();
  qDebug() << "friendId" << fId;
  tox->sendFriendRequest(fId, nick, message);

  //    uint32_t friendNumber = tox_friend_add(tox.get(), friendId.getBytes(),
  //    cMessage.data(), cMessage.size(), nullptr);
  //    if (friendNumber == std::numeric_limits<uint32_t>::max()) {
  //        qDebug() << "Failed to request friendship";
  //        emit failedToAddFriend(friendPk);
  //    } else {
  //        qDebug() << "Requested friendship of " << friendNumber;
  //        emit friendAdded(friendNumber, friendPk);
  //        emit requestSent(friendPk, message);
  //    }

  emit saveRequest();
}

bool Core::sendMessageWithType(QString friendId, const QString &message,
                               Tox_Message_Type type, ReceiptNum &receipt,
                               bool encrypt) {

  qDebug() << "Core::sendMessageWithType:" << message
           << "=>friend:" << friendId;
  QString receipts = 0;
  bool yes = tox->sendToFriend(friendId, message, receipts, encrypt);

  //  int size = message.toUtf8().size();
  //  auto maxSize = tox_max_message_length();
  //  if (size > maxSize) {
  //    qCritical() << "Core::sendMessageWithType called with message of size:"
  //                << size << "when max is:" << maxSize << ". Ignoring.";
  //    return false;
  //  }
  //
  //  ToxString cMessage(message);
  //  Tox_Err_Friend_Send_Message error;
  receipt = ReceiptNum{receipts};
  //  if (parseFriendSendMessageError(error)) {
  //    return true;
  //  }
  return yes;
}

bool Core::sendMessage(QString friendId, const QString &message,
                       ReceiptNum &receipt, bool encrypt) {
  QMutexLocker ml(&coreLoopLock);
  return sendMessageWithType(friendId, message, TOX_MESSAGE_TYPE_NORMAL,
                             receipt, encrypt);
}

bool Core::sendAction(QString friendId, const QString &action,
                      ReceiptNum &receipt, bool encrypt) {
  QMutexLocker ml(&coreLoopLock);
  return sendMessageWithType(friendId, action, TOX_MESSAGE_TYPE_ACTION, receipt,
                             encrypt);
}

void Core::sendTyping(QString friendId, bool typing) {
  QMutexLocker ml{&coreLoopLock};
  tox->sendChatState(friendId, typing ? 2 : 4);
  emit failedToSetTyping(typing);
}

void Core::sendGroupMessageWithType(QString groupId, const QString &message,
                                    Tox_Message_Type type) {
  QMutexLocker ml{&coreLoopLock};
  QString r = 0;

  tox->sendToGroup(groupId, message, r);

  //  int size = message.toUtf8().size();
  //  auto maxSize = tox_max_message_length();
  //  if (size > maxSize) {
  //    qCritical() << "Core::sendMessageWithType called with message of size:"
  //                << size << "when max is:" << maxSize << ". Ignoring.";
  //    return;
  //  }
  //
  //  ToxString cMsg(message);
  //  Tox_Err_Conference_Send_Message error;
  //  tox_conference_send_message(tox.get(), groupId, type, cMsg.data(),
  //                              cMsg.size(), &error);
  //  if (!parseConferenceSendMessageError(error)) {
  //    emit groupSentFailed(groupId);
  //    return;
  //  }
}

void Core::sendGroupMessage(QString groupId, const QString &message) {
  QMutexLocker ml{&coreLoopLock};
  sendGroupMessageWithType(groupId, message, TOX_MESSAGE_TYPE_NORMAL);
}

void Core::sendGroupAction(QString groupId, const QString &message) {
  QMutexLocker ml{&coreLoopLock};
  sendGroupMessageWithType(groupId, message, TOX_MESSAGE_TYPE_ACTION);
}

void Core::changeGroupTitle(QString groupId, const QString &title) {
  QMutexLocker ml{&coreLoopLock};
  tox->setRoomName(groupId, title);

  //
  //  ToxString cTitle(title);
  //  Tox_Err_Conference_Title error;
  //  bool success = tox_conference_set_title(tox.get(), groupId, cTitle.data(),
  //                                          cTitle.size(), &error);
  //    if (success) {
  emit saveRequest();
  emit groupTitleChanged(groupId, getUsername(), title);
  //      return;
  //    }
  //
  //  qCritical() << "Fail of tox_conference_set_title";
  //  switch (error) {
  //  case TOX_ERR_CONFERENCE_TITLE_CONFERENCE_NOT_FOUND:
  //    qCritical() << "Conference not found";
  //    break;
  //
  //  case TOX_ERR_CONFERENCE_TITLE_FAIL_SEND:
  //    qCritical() << "Conference title failed to send";
  //    break;
  //
  //  case TOX_ERR_CONFERENCE_TITLE_INVALID_LENGTH:
  //    qCritical() << "Invalid length";
  //    break;
  //
  //  default:
  //    break;
  //  }
}

void Core::removeFriend(QString friendId) {
  qDebug() << "removeFriend" << friendId;
  QMutexLocker ml{&coreLoopLock};
  bool success = tox->removeFriend(friendId);
  if (success) {
    emit saveRequest();
    emit friendRemoved(friendId);
    return;
  }
}

void Core::leaveGroup(QString groupId) {
  bool success = tox->leaveGroup(groupId);
  if (success) {
    emit saveRequest();
    av->leaveGroupCall(groupId);
    return;
  }
}

void Core::destroyGroup(QString groupId) {
  bool success = tox->destroyGroup(groupId);
  if (success) {
    emit saveRequest();
    av->leaveGroupCall(groupId);
    return;
  }
}


/**
 * @brief Returns our username, or an empty string on failure
 */
QString Core::getUsername() const {
  QMutexLocker ml{&coreLoopLock};
  QString sname = tox->getSelfUsername();
  return sname;
}

void Core::setUsername(const QString &username) {
  QMutexLocker ml{&coreLoopLock};

  if (username == getUsername()) {
    return;
  }

  tox->setSelfNickname(username);

  emit usernameSet(username);
  emit saveRequest();
}

QString Core::getNick() const {
  QMutexLocker ml{&coreLoopLock};
  return tox->getSelfNick();
}

void Core::setPassword(const QString &password) {
  QMutexLocker ml{&coreLoopLock};

  if (password.isEmpty()) {
    return;
  }

  tox->changePassword(password);
}

/**
 * @brief Returns our Ok ID
 */
ToxId Core::getSelfId() const {
  QMutexLocker ml{&coreLoopLock};
  auto selfId = tox->getSelfId();
  return ToxId(selfId.toString().toUtf8());
}

/**
 * @brief Gets self public key
 * @return Self PK
 */
ToxPk Core::getSelfPublicKey() const {
  QMutexLocker ml{&coreLoopLock};
  auto friendId = tox->getSelfId();
  return ToxPk(friendId);
}

/**
 * @brief Returns our public and private keys
 */
QPair<QByteArray, QByteArray> Core::getKeypair() const {
  QMutexLocker ml{&coreLoopLock};

  QPair<QByteArray, QByteArray> keypair;
  //  assert(tox != nullptr);
  //
  //  QByteArray pk(TOX_PUBLIC_KEY_SIZE, 0x00);
  //  QByteArray sk(TOX_SECRET_KEY_SIZE, 0x00);
  //  tox_self_get_public_key(tox.get(), reinterpret_cast<uint8_t
  //  *>(pk.data())); tox_self_get_secret_key(tox.get(),
  //  reinterpret_cast<uint8_t *>(sk.data())); keypair.first = pk;
  //  keypair.second = sk;
  return keypair;
}

/**
 * @brief Returns our status message, or an empty string on failure
 */
QString Core::getStatusMessage() const {
  QMutexLocker ml{&coreLoopLock};
  auto status = getStatus();
  return Status::getAssetSuffix(status);
}

/**
 * @brief Returns our user status
 */
Status::Status Core::getStatus() const {
  assert(tox != nullptr);
  QMutexLocker ml{&coreLoopLock};
  switch (tox->getSelfStatus()) {
  case TOX_USER_STATUS_Available:
  case TOX_USER_STATUS_Chat:
    return Status::Status::Online;
  case TOX_USER_STATUS_Away:
  case TOX_USER_STATUS_XA:
    return Status::Status::Away;
  case TOX_USER_STATUS_DND:
    return Status::Status::Busy;
  default:
    return Status::Status::Offline;
  }
}

void Core::setStatusMessage(const QString &message) {
  QMutexLocker ml{&coreLoopLock};

  if (message == getStatusMessage()) {
    return;
  }

  //  ToxString cMessage(message);
  //  if (!tox_self_set_status_message(tox.get(), cMessage.data(),
  //  cMessage.size(), nullptr)) {
  //    emit failedToSetStatusMessage(message);
  //    return;
  //  }

  emit saveRequest();
  emit statusMessageSet(message);
}

void Core::setStatus(Status::Status status) {
  QMutexLocker ml{&coreLoopLock};

  Tox_User_Status userstatus;
  switch (status) {
  case Status::Status::Online:
    userstatus = Tox_User_Status ::TOX_USER_STATUS_Available;
    break;

  case Status::Status::Away:
    userstatus = TOX_USER_STATUS_Away;
    break;

  case Status::Status::Busy:
    userstatus = TOX_USER_STATUS_DND;
    break;

  default:
    return;
    break;
  }
  //  tox_self_set_status(tox.get(), userstatus);
  emit saveRequest();
  emit statusSet(status);
}

void Core::setAvatar(const QByteArray &avatar) { tox->setSelfAvatar(avatar); }

/**
 * @brief Returns the unencrypted tox save data
 */
QByteArray Core::getToxSaveData() {
  QMutexLocker ml{&coreLoopLock};

  QByteArray data;
  //  uint32_t fileSize = tox_get_savedata_size(tox.get());
  //  data.resize(fileSize);
  //  tox_get_savedata(tox.get(), (uint8_t *)data.data());
  return data;
}

// Declared to avoid code duplication
#define GET_FRIEND_PROPERTY(property, function, checkSize)                     \
  const size_t property##Size = function##_size(tox.get(), ids[i], nullptr);   \
  if ((!checkSize || property##Size) && property##Size != SIZE_MAX) {          \
    uint8_t *prop = new uint8_t[property##Size];                               \
    if (function(tox.get(), ids[i], prop, nullptr)) {                          \
      QString propStr = ToxString(prop, property##Size).getQString();          \
      emit friend##property##Changed(ids[i], propStr);                         \
    }                                                                          \
                                                                               \
    delete[] prop;                                                             \
  }

void Core::loadFriends() {
  QMutexLocker ml{&coreLoopLock};
  const size_t friendCount = tox->getFriendCount();
  qDebug() << "friendCount" << friendCount;
  if (friendCount == 0) {
    return;
  }

  //  std::list<lib::IM::FriendId> peers = messenger->loadFriendList();
  //  for (auto itr : peers) {
  //    qDebug() << "id=" << qstring(itr.getJid())
  //             << " name=" << qstring(itr.getUsername());
  //  }

  // uint32_t *ids = new uint32_t[friendCount];
  // tox_self_get_friend_list(tox.get(), ids);
  // uint8_t friendPk[TOX_PUBLIC_KEY_SIZE] = {0x00};
  // for (size_t i = 0; i < friendCount; ++i) {
  //   if (!tox_friend_get_public_key(tox.get(), ids[i], friendPk, nullptr)) {
  //     continue;
  //   }

  //   emit friendAdded(ids[i], ToxPk(friendPk));
  //   GET_FRIEND_PROPERTY(Username, tox_friend_get_name, true);
  //   GET_FRIEND_PROPERTY(StatusMessage, tox_friend_get_status_message, false);
  //   checkLastOnline(ids[i]);
  // }
  // delete[] ids;
}

void Core::loadGroups() {
  QMutexLocker ml{&coreLoopLock};

  //  const size_t groupCount = tox_conference_get_chatlist_size(tox.get());
  //  if (groupCount == 0) {
  //    return;
  //  }
  //
  //  auto groupNumbers = new uint32_t[groupCount];
  //  tox_conference_get_chatlist(tox.get(), groupNumbers);
  //
  //  for (size_t i = 0; i < groupCount; ++i) {
  //    TOX_ERR_CONFERENCE_TITLE error;
  //    QString name;
  //    const auto groupNumber = groupNumbers[i];
  //    size_t titleSize =
  //        tox_conference_get_title_size(tox.get(), groupNumber, &error);
  //    const GroupId persistentId = getGroupPersistentId(groupNumber);
  //    const QString defaultName =
  //        tr("Groupchat %1").arg(persistentId.toString().left(8));
  //    if (LogConferenceTitleError(error)) {
  //      name = defaultName;
  //    } else {
  //      QByteArray nameByteArray =
  //          QByteArray(static_cast<int>(titleSize), Qt::Uninitialized);
  //      tox_conference_get_title(
  //          tox.get(), groupNumber,
  //          reinterpret_cast<uint8_t *>(nameByteArray.data()), &error);
  //      if (LogConferenceTitleError(error)) {
  //        name = defaultName;
  //      } else {
  //        name = ToxString(nameByteArray).getQString();
  //      }
  //    }
  //    if (getGroupAvEnabled(groupNumber)) {
  //      if (toxav_groupchat_enable_av(tox.get(), groupNumber,
  //                                    CoreAV::groupCallCallback, this)) {
  //        qCritical() << "Failed to enable audio on loaded group" <<
  //        groupNumber;
  //      }
  //    }
  //    emit emptyGroupCreated(groupNumber, persistentId, name);
  //  }

  //  delete[] groupNumbers;
}

void Core::checkLastOnline(QString friendId) {
  QMutexLocker ml{&coreLoopLock};

  //  const uint64_t lastOnline =
  //      tox_friend_get_last_online(tox.get(), friendId, nullptr);
  //  if (lastOnline != std::numeric_limits<uint64_t>::max()) {
  //    emit friendLastSeenChanged(friendId, QDateTime::fromTime_t(lastOnline));
  //  }
}

/**
 * @brief Returns the list of friendIds in our friendlist, an empty list on
 * error
 */
QVector<ToxPk> Core::loadFriendList() const {
  QMutexLocker ml{&coreLoopLock};

  std::list<lib::messenger::FriendId> friendList = tox->getFriendList();
  QVector<ToxPk> friends;
  for (auto &p : friendList) {
    friends.push_back(ToxPk(p.toString()));
  }
  return friends;
}

/**
 * @brief Print in console text of error.
 * @param error Error to handle.
 * @return True if no error, false otherwise.
 */
bool Core::parsePeerQueryError(Tox_Err_Conference_Peer_Query error) const {
  switch (error) {
  case TOX_ERR_CONFERENCE_PEER_QUERY_OK:
    return true;

  case TOX_ERR_CONFERENCE_PEER_QUERY_CONFERENCE_NOT_FOUND:
    qCritical() << "Conference not found";
    return false;

  case TOX_ERR_CONFERENCE_PEER_QUERY_NO_CONNECTION:
    qCritical() << "No connection";
    return false;

  case TOX_ERR_CONFERENCE_PEER_QUERY_PEER_NOT_FOUND:
    qCritical() << "Peer not found";
    return false;

  default:
    qCritical() << "Unknow error code:" << error;
    return false;
  }
}

GroupId Core::getGroupPersistentId(QString groupId) const {
  QMutexLocker ml{&coreLoopLock};
  return GroupId{groupId.toUtf8()};
}

/**
 * @brief Get number of peers in the conference.
 * @return The number of peers in the conference. UINT32_MAX on failure.
 */
uint32_t Core::getGroupNumberPeers(QString groupId) const {
  QMutexLocker ml{&coreLoopLock};
  qDebug() << "getGroupNumberPeers:" << groupId;

  //  Tox_Err_Conference_Peer_Query error;
  //  uint32_t count = tox_conference_peer_count(tox.get(), groupId, &error);
  //  if (!parsePeerQueryError(error)) {
  //    return std::numeric_limits<uint32_t>::max();
  //  }

  //  return count;
  return 0;
}

/**
 * @brief Get the name of a peer of a group
 */
QString Core::getGroupPeerName(QString groupId, QString peerId) const {
  QMutexLocker ml{&coreLoopLock};

  // from tox.h: "If peer_number == UINT32_MAX, then author is unknown (e.g.
  // initial joining the conference)."
  if (peerId == std::numeric_limits<uint32_t>::max()) {
    return {};
  }

  //  Tox_Err_Conference_Peer_Query error;
  //  size_t length =
  //      tox_conference_peer_get_name_size(tox.get(), groupId, peerId, &error);
  //  if (!parsePeerQueryError(error)) {
  //    return QString{};
  //  }
  //
  //  QByteArray name(length, Qt::Uninitialized);
  //  uint8_t *namePtr = reinterpret_cast<uint8_t *>(name.data());
  //  bool success =
  //      tox_conference_peer_get_name(tox.get(), groupId, peerId, namePtr,
  //      &error);
  //  if (!parsePeerQueryError(error)) {
  //    return QString{};
  //  }
  //  assert(success);

  //  return ToxString(name).getQString();
  return QString{};
}

/**
 * @brief Get the public key of a peer of a group
 */
ToxPk Core::getGroupPeerPk(QString groupId, QString peerId) const {
  QMutexLocker ml{&coreLoopLock};

  //  uint8_t friendPk[TOX_PUBLIC_KEY_SIZE] = {0x00};
  //  Tox_Err_Conference_Peer_Query error;
  //  bool success = tox_conference_peer_get_public_key(tox.get(), groupId,
  //  peerId,
  //                                                    friendPk, &error);
  //  if (!parsePeerQueryError(error)) {
  //    return ToxPk{};
  //  }
  //  assert(success);

  auto toxPk = ToxPk{lib::messenger::PeerId(peerId)};

  //  return ToxPk(friendPk);
  return toxPk;
}

/**
 * @brief Get the names of the peers of a group
 */
QStringList Core::getGroupPeerNames(QString groupId) const {
  QMutexLocker ml{&coreLoopLock};

  assert(tox != nullptr);

  uint32_t nPeers = getGroupNumberPeers(groupId);
  if (nPeers == std::numeric_limits<uint32_t>::max()) {
    qWarning() << "getGroupPeerNames: Unable to get number of peers";
    return {};
  }

  QStringList names;
  //  for (uint32_t i = 0; i < nPeers; ++i) {
  //    TOX_ERR_CONFERENCE_PEER_QUERY error;
  //    size_t length =
  //        tox_conference_peer_get_name_size(tox.get(), groupId, i, &error);
  //    if (!parsePeerQueryError(error)) {
  //      names.append(QString());
  //      continue;
  //    }
  //
  //    QByteArray name(length, Qt::Uninitialized);
  //    uint8_t *namePtr = reinterpret_cast<uint8_t *>(name.data());
  //    bool ok =
  //        tox_conference_peer_get_name(tox.get(), groupId, i, namePtr,
  //        &error);
  //    if (ok && parsePeerQueryError(error)) {
  //      names.append(ToxString(name).getQString());
  //    } else {
  //      names.append(QString());
  //    }
  //  }

  //  names.append("user1");
  //  names.append("user2");
  //  assert(names.size() == nPeers);
  return names;
}

/**
 * @brief Check, that group has audio or video stream
 * @param groupId Id of group to check
 * @return True for AV groups, false for text-only groups
 */
bool Core::getGroupAvEnabled(QString groupId) const {
  QMutexLocker ml{&coreLoopLock};
  //  TOX_ERR_CONFERENCE_GET_TYPE error;
  //  TOX_CONFERENCE_TYPE type =
  //      tox_conference_get_type(tox.get(), groupId, &error);
  //  switch (error) {
  //  case TOX_ERR_CONFERENCE_GET_TYPE_OK:
  //    break;
  //  case TOX_ERR_CONFERENCE_GET_TYPE_CONFERENCE_NOT_FOUND:
  //    qWarning() << "Conference not found";
  //    break;
  //  default:
  //    qWarning() << "Unknown error code:" << QString::number(error);
  //    break;
  //  }

  //  return type == TOX_CONFERENCE_TYPE_AV;
  return true;
}

/**
 * @brief Print in console text of error.
 * @param error Error to handle.
 * @return True if no error, false otherwise.
 */
bool Core::parseConferenceJoinError(Tox_Err_Conference_Join error) const {
  switch (error) {
  case TOX_ERR_CONFERENCE_JOIN_OK:
    return true;

  case TOX_ERR_CONFERENCE_JOIN_DUPLICATE:
    qCritical() << "Conference duplicate";
    return false;

  case TOX_ERR_CONFERENCE_JOIN_FAIL_SEND:
    qCritical() << "Conference join failed to send";
    return false;

  case TOX_ERR_CONFERENCE_JOIN_FRIEND_NOT_FOUND:
    qCritical() << "Friend not found";
    return false;

  case TOX_ERR_CONFERENCE_JOIN_INIT_FAIL:
    qCritical() << "Init fail";
    return false;

  case TOX_ERR_CONFERENCE_JOIN_INVALID_LENGTH:
    qCritical() << "Invalid length";
    return false;

  case TOX_ERR_CONFERENCE_JOIN_WRONG_TYPE:
    qCritical() << "Wrong conference type";
    return false;

  default:
    qCritical() << "Unknow error code:" << error;
    return false;
  }
}

/**
 * @brief Accept a groupchat invite.
 * @param inviteInfo Object which contains info about group invitation
 *
 * @return Conference number on success, UINT32_MAX on failure.
 */
QString Core::joinGroupchat(const GroupInvite &inviteInfo) {
  QMutexLocker ml{&coreLoopLock};

  tox->joinGroup(inviteInfo.getGroupId());

  //  const QString friendId = inviteInfo.getFriendId();
  //  const uint8_t confType = inviteInfo.getType();
  //  const QByteArray invite = inviteInfo.getInvite();
  //  const uint8_t *const cookie = reinterpret_cast<const uint8_t
  //  *>(invite.data()); const size_t cookieLength = invite.length(); QString
  //  groupNum{std::numeric_limits<uint32_t>::max()};

  //  switch (confType) {
  //  case TOX_CONFERENCE_TYPE_TEXT: {
  //    qDebug() << QString(
  //                    "Trying to join text groupchat invite sent by friend
  //                    %1") .arg(friendId);
  //    Tox_Err_Conference_Join error;
  //    groupNum = tox_conference_join(tox.get(), friendId, cookie,
  //    cookieLength, &error); if (!parseConferenceJoinError(error)) {
  //      groupNum = std::numeric_limits<uint32_t>::max();
  //    }
  //    break;
  //  }
  //  case TOX_CONFERENCE_TYPE_AV: {
  //    qDebug() << QString("Trying to join AV groupchat invite sent by friend
  //    %1")
  //                    .arg(friendId);
  //    groupNum = toxav_join_av_groupchat(tox.get(), friendId, cookie,
  //                                       cookieLength,
  //                                       CoreAV::groupCallCallback,
  //                                       const_cast<Core *>(this));
  //    break;
  //  }
  //  default:
  //    qWarning() << "joinGroupchat: Unknown groupchat type " << confType;
  //  }
  //  if (groupNum != std::numeric_limits<uint32_t>::max()) {
  //    emit saveRequest();
  //    emit groupJoined(groupNum, getGroupPersistentId(groupNum));
  //  }
  //  return groupNum;
  return {};
}

void Core::joinRoom(const QString &groupId) {
  tox->joinGroup(groupId);
}

void Core::groupInviteFriend(QString friendId, QString groupId) {
  QMutexLocker ml{&coreLoopLock};
  tox->inviteGroup(groupId, friendId);

  //
  //  Tox_Err_Conference_Invite error;
  //  //  tox_conference_invite(tox.get(), friendId, groupId, &error);
  //
  //  switch (error) {
  //  case TOX_ERR_CONFERENCE_INVITE_OK:
  //    break;
  //
  //  case TOX_ERR_CONFERENCE_INVITE_CONFERENCE_NOT_FOUND:
  //    qCritical() << "Conference not found";
  //    break;
  //
  //  case TOX_ERR_CONFERENCE_INVITE_FAIL_SEND:
  //    qCritical() << "Conference invite failed to send";
  //    break;
  //
  //  default:
  //    break;
  //  }
}

QString Core::createGroup(uint8_t type) {
  QMutexLocker ml{&coreLoopLock};
  QString id = QUuid::createUuid().toString(QUuid::StringFormat::WithoutBraces);
  tox->createGroup(id.split("-").at(0));
  return id;
  //    if (type == TOX_CONFERENCE_TYPE_TEXT) {
  //      Tox_Err_Conference_New error;
  //      QString groupId = tox_conference_new(tox.get(), &error);
  //      switch (error) {
  //      case TOX_ERR_CONFERENCE_NEW_OK:
  //        emit saveRequest();
  //        emit emptyGroupCreated(groupId, getGroupPersistentId(groupId));
  //        return groupId;
  //
  //      case TOX_ERR_CONFERENCE_NEW_INIT:
  //        qCritical() << "The conference instance failed to initialize";
  //        return std::numeric_limits<uint32_t>::max();
  //
  //      default:
  //        return std::numeric_limits<uint32_t>::max();
  //      }
  //    } else if (type == TOX_CONFERENCE_TYPE_AV) {
  //      QString groupId =
  //          toxav_add_av_groupchat(tox.get(), CoreAV::groupCallCallback,
  //          this);
  //      emit saveRequest();
  //      emit emptyGroupCreated(groupId, getGroupPersistentId(groupId));
  //      return groupId;
  //    } else {
  //      qWarning() << "createGroup: Unknown type " << type;
  //      return -1;
  //    }
}

/**
 * @brief Checks if a friend is online. Unknown friends are considered offline.
 */
bool Core::isFriendOnline(QString friendId) const {
  QMutexLocker ml{&coreLoopLock};
  //
  //  Tox_Connection connection =
  //      tox_friend_get_connection_status(tox.get(), friendId, nullptr);
  //  return connection != TOX_CONNECTION_NONE;
  return false;
}

/**
 * @brief Checks if we have a friend by public key
 */
bool Core::hasFriendWithPublicKey(const ToxPk &publicKey) const {
  //  QMutexLocker ml{&coreLoopLock};
  //
  //  if (publicKey.isEmpty()) {
  //    return false;
  //  }
  //
  //  QString friendId = tox_friend_by_public_key(tox.get(),
  //  publicKey.getData(), nullptr); return friendId !=
  //  std::numeric_limits<uint32_t>::max();
  return false;
}

/**
 * @brief Get the public key part of the ToxID only
 */
ToxPk Core::getFriendPublicKey(QString friendNumber) const {
  //qDebug() << "getFriendPublicKey" << friendNumber;
  //  QMutexLocker ml{&coreLoopLock};
  //  uint8_t rawid[TOX_PUBLIC_KEY_SIZE];
  //  if (!tox_friend_get_public_key(tox.get(), friendNumber, rawid, nullptr)) {
  //    qWarning() << "getFriendPublicKey: Getting public key failed";
  //    return ToxPk();
  //  }
  return ToxPk(friendNumber.toUtf8());
}

/**
 * @brief Get the username of a friend
 */
QString Core::getFriendUsername(QString friendnumber) const {
  return tox->getSelfUsername();
}

void Core::getFriendInfo(const QString& friendnumber) const {
  tox->getFriendVCard(friendnumber);
}

QStringList Core::splitMessage(const QString &message) {
  QStringList splittedMsgs;
  QByteArray ba_message{message.toUtf8()};

  /*
   * TODO: Remove this hack; the reported max message length we receive from
   * c-toxcore as of 08-02-2019 is inaccurate, causing us to generate too large
   * messages when splitting them up.
   *
   * The inconsistency lies in c-toxcore group.c:2480 using
   * MAX_GROUP_MESSAGE_DATA_LEN to verify message size is within limit, but
   * tox_max_message_length giving a different size limit to us.
   *
   * (uint32_t tox_max_message_length(void); declared in tox.h, unable to see
   * explicit definition)
   */
  const auto maxLen = 1024;

  while (ba_message.size() > maxLen) {
    int splitPos = ba_message.lastIndexOf('\n', maxLen - 1);

    if (splitPos <= 0) {
      splitPos = ba_message.lastIndexOf(' ', maxLen - 1);
    }

    if (splitPos <= 0) {
      constexpr uint8_t firstOfMultiByteMask = 0xC0;
      constexpr uint8_t multiByteMask = 0x80;
      splitPos = maxLen;
      // don't split a utf8 character
      if ((ba_message[splitPos] & multiByteMask) == multiByteMask) {
        while ((ba_message[splitPos] & firstOfMultiByteMask) !=
               firstOfMultiByteMask) {
          --splitPos;
        }
      }
      --splitPos;
    }
    splittedMsgs.append(QString{ba_message.left(splitPos + 1)});
    ba_message = ba_message.mid(splitPos + 1);
  }

  splittedMsgs.append(QString{ba_message});
  return splittedMsgs;
}

QString Core::getPeerName(const ToxPk &id) const {
  QMutexLocker ml{&coreLoopLock};

  QString name;
  //  QString friendId = tox_friend_by_public_key(tox.get(), id.getData(),
  //  nullptr); if (friendId == std::numeric_limits<uint32_t>::max()) {
  //    qWarning() << "getPeerName: No such peer";
  //    return name;
  //  }
  //
  //  const size_t nameSize =
  //      tox_friend_get_name_size(tox.get(), friendId, nullptr);
  //  if (nameSize == SIZE_MAX) {
  //    return name;
  //  }
  //
  //  uint8_t *cname =
  //      new uint8_t[nameSize < tox_max_name_length() ? tox_max_name_length()
  //                                                   : nameSize];
  //  if (!tox_friend_get_name(tox.get(), friendId, cname, nullptr)) {
  //    qWarning() << "getPeerName: Can't get name of friend " +
  //                      QString().setNum(friendId);
  //    delete[] cname;
  //    return name;
  //  }
  //
  //  name = ToxString(cname, nameSize).getQString();
  //  delete[] cname;
  return name;
}

void Core::logout() { tox->stop(); }

void Core::onSelfNameChanged(QString name) {
  QMutexLocker ml{&coreLoopLock};
  qDebug()<<"onSelfNameChanged:"<<name;
  emit usernameSet(name);
}

void Core::onSelfAvatarChanged(const std::string avatar) {
  QMutexLocker ml{&coreLoopLock};
  emit avatarSet(QByteArray::fromStdString(avatar));
}

void Core::onSelfStatusChanged(Tox_User_Status userStatus,
                               const std::string &msg) {
  QMutexLocker ml{&coreLoopLock};
  auto st = fromToxStatus(userStatus);

  emit statusSet(st);
  auto t = getTitle(st);
  if (!msg.empty()) {
    t.append(": ").append(msg.data());
  }
  emit statusMessageSet(t);
}

void Core::onSelfIdChanged(QString id) {
  QMutexLocker ml{&coreLoopLock};
  emit idSet(ToxId(id));
}

void Core::sendReceiptReceived(const QString &friendId, QString receipt) {
  qDebug() << "friendId" << friendId << "receipt" << receipt;
  tox->receiptReceived(friendId, receipt);
}

void Core::requestBookmarks() {
  tox->requestBookmarks();
}

void Core::setUIStarted() {
  tox->setUIStarted();
}
void Core::loadGroupList() const {
  tox->loadGroupList();
}
