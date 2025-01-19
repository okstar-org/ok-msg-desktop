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
// Created by gaojie on 25-1-9.
//

#include "profile.h"
#include "application.h"
#include "base/identicon.h"
#include "base/images.h"

#include "settings.h"
namespace module::im {

Profile::Profile(const QString& host,
                 const QString& name,
                 const QString& password,
                 bool isNewProfile) {
    qDebug() << "Initialize profile for" << name;

    auto app = ok::Application::Instance();

    _profile = app->getProfile();
    // connect(_profile, &lib::session::Profile::nickChanged, this, &Profile::nickChanged);

    storageManager = _profile->create(_profile->getUsername());
    okSettings = storageManager->getGlobalSettings();
    db = std::shared_ptr<lib::db::RawDatabase>(storageManager->createDatabase(OK_IM_MODULE));
    // TODO 待优化
    history.reset(new History(db));
    s = std::make_unique<Settings>(storageManager->createSetting(OK_IM_MODULE));
    initCore(s.get());
}

/**
 * @brief Creates a new profile and the associated Core* instance.
 * @param name Username.
 * @param password If password is not empty, the profile will be encrypted.
 * @return Returns a nullptr on error. Profile pointer otherwise.
 *
 * @note If the profile is already in use return nullptr.
 */
std::unique_ptr<Profile> Profile::createProfile(QString host, QString name, QString password) {
    return std::make_unique<Profile>(host, name, password, true);
}

/**
 * @brief Get our avatar from cache.
 * @return Avatar as QPixmap.
 */
const QPixmap& Profile::loadAvatar() {
    auto buf = _profile->getAvatar();
    if (!buf.isEmpty()) {
        avatar.loadFromData(buf);
        return avatar;
    }

    if (avatar.isNull()) {
        // 获取默认
        if (s->getShowIdenticons()) {
            avatar = QPixmap::fromImage(
                    Identicon(core->getSelfPeerId().getPublicKey().getByteArray()).toImage(16));
        }
    }
    return avatar;
}

void Profile::saveContactAlias(const QString& contactId, const QString& alias) {
    history->setPeerAlias(contactId, alias);
    core->setGroupAlias(contactId, alias);
    emit contactAliasChanged(contactId, alias);
}

QString Profile::getContactAlias(const QString& contactId) {
    return history->getPeerAlias(contactId);
}

uint Profile::addContact(const ContactId& cid) {
    qDebug() << __func__ << cid.toString();
    return history->addNewContact(cid.toString());
}

Core* Profile::getCore() {
    return core.get();
}

void Profile::initCore(ICoreSettings* s) {
    auto& sign = _profile->getSignIn();

    Core::ToxCoreErrors err;
    core = Core::makeToxCore(_profile->getMessenger(), s, &err);
    if (!core) {
        switch (err) {
            case Core::ToxCoreErrors::BAD_PROXY:
                emit badProxy();
                break;
            case Core::ToxCoreErrors::ERROR_ALLOC:
            case Core::ToxCoreErrors::FAILED_TO_START:
            case Core::ToxCoreErrors::INVALID_SAVE:
            default:
                emit failedToStart();
        }

        qDebug() << "failed to start ToxCore";
        return;
    }

    // save tox file when Core requests it
    connect(core.get(), &Core::saveRequest, this, &Profile::onSaveToxSave);
    // react to avatar changes
    connect(core.get(), &Core::friendAvatarRemoved, this, &Profile::removeFriendAvatar);
    connect(core.get(), &Core::friendAvatarChanged, this, &Profile::setFriendAvatar);
    //    connect(core.get(), &Core::fileAvatarOfferReceived, this, &Profile::onAvatarOfferReceived,
    //            Qt::ConnectionType::QueuedConnection);
    connect(core.get(), &Core::started, this, [this]() { emit selfAvatarChanged(loadAvatar()); });

    connect(core.get(), &Core::vCardSet, [&](const VCard& vCard) { setVCard(vCard); });
    connect(core.get(), &Core::avatarSet, this, &Profile::onAvatarSet);

    // CoreAV
    coreAv = CoreAV::makeCoreAV(core.get());

    // CoreFile
    coreFile = CoreFile::makeCoreFile(core.get());
}

void Profile::start() {
    qDebug() << __func__;
    core->start();
    coreAv->start();
    coreFile->start();
}

void Profile::stop() {
    qDebug() << __func__;
    core->stop();
}

void Profile::setAvatar(QByteArray& pic, bool saveToCore) {
    _profile->setAvatar(pic);
    emit selfAvatarChanged(loadAvatar());
    if (saveToCore) {
        core->setAvatar(pic);
    }
}

void Profile::setAvatarOnly(const QPixmap& pic) {
    QByteArray byteArray;
    ok::base::Images::loadFormPixmap(pic, &byteArray);
    setAvatar(byteArray, false);
    emit selfAvatarChanged(pic);
}

bool Profile::removeAvatar(bool saveToCore) {
    _profile->removeAvatar();
    emit selfAvatarChanged({});
    if (saveToCore) core->setAvatar({});
    return true;
}

QByteArray Profile::loadAvatarData(const ContactId& friendId) {
    return _profile->loadAvatarData(ok::base::Jid(friendId.toString()));
}

QPixmap Profile::loadAvatar(const ContactId& friendId) {
    QPixmap map;
    auto y = ok::base::Images::putToPixmap(loadAvatarData(friendId), map);
    if (!y) qWarning() << "Can not load image to QPixmap";
    return map;
}

void Profile::setFriendAvatar(const ContactId& owner, const QByteArray& pic) {
    _profile->saveFriendAvatar(ok::base::Jid(owner.toString()), pic);
}

void Profile::removeFriendAvatar(const ContactId& owner) {
    _profile->removeFriendAvatar(ok::base::Jid(owner.toString()));
}

const QString& Profile::getUsername() {
    return _profile->getUsername();
}

History* Profile::getHistory() {
    return history.get();
}

const QString& Profile::getFullName() {
    if (!vCard.fullName.isEmpty()) {
        return vCard.fullName;
    }
    return _profile->getUsername();
}

void Profile::setNick(const QString& nick_, bool saveToCore) {
    if (vCard.nickname != nick_) {
        vCard.nickname = nick_;
    };

    _profile->setNickname(nick_);
    if (saveToCore) core->setNick(nick_);
}

void Profile::setVCard(const VCard& v) {
    vCard = v;
    emit vCardChanged(vCard);
}

/**
 * @brief Saves the profile's .tox save, encrypted if needed.
 * @warning Invalid on deleted profiles.
 */
void Profile::onSaveToxSave() {
    QByteArray data = core->getToxSaveData();
    if (!data.size()) {
        return;
    }
    // assert(data.size());
    //    saveToxSave(data);
}

QString Profile::setPassword(const QString& pwd) {
    return QString();
}
QString Profile::getHost() {
    return _profile->getSignIn().host;
}

Settings* Profile::getSettings() const {
    return s.get();
}

void Profile::quit() {
    s->saveGlobal();
    s->sync();
}

void Profile::onAvatarSet(QByteArray avatar) {
    setAvatar(avatar, false);
}
}  // namespace module::im