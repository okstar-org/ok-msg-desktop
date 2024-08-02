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

#include "toxid.h"
#include "FriendId.h"

#include <QDebug>
#include <QRegularExpression>
#include <cstdint>

// Tox doesn't publicly define these
#define NOSPAM_BYTES 4
#define CHECKSUM_BYTES 2

#define PUBLIC_KEY_HEX_CHARS (2 * TOX_PUBLIC_KEY_SIZE)
#define NOSPAM_HEX_CHARS (2 * NOSPAM_BYTES)
#define CHECKSUM_HEX_CHARS (2 * CHECKSUM_BYTES)
#define TOXID_HEX_CHARS (2 * TOX_ADDRESS_SIZE)

// const QRegularExpression ToxId::ToxIdRegEx(
//     QString("(^|\\s)[A-Fa-f0-9]{%1}($|\\s)").arg(TOXID_HEX_CHARS));

/**
 * @class ToxId
 * @brief This class represents a Tox ID.
 *
 * @endcode
 */

/**
 * @brief The default constructor. Creates an empty Tox ID.
 */
ToxId::ToxId() : toxId() {}

/**
 * @brief The copy constructor.
 * @param other ToxId to copy
 */
ToxId::ToxId(const ToxId& other) : toxId(other.toxId) {}

/**
 * @brief Create a Tox ID from a QString.
 *
 * If the given rawId is not a valid Ok ID, but can be a Public Key then:
 * publicKey == rawId and noSpam == 0 == checkSum.
 * If the given rawId isn't a valid Public Key or Ok ID a ToxId with all zero
 * bytes is created.
 *
 * @param id Ok ID string to convert to ToxId object
 */
ToxId::ToxId(const QString& id) {
    //  qDebug() << "ToxId::ToxId (id)" << id;
    // TODO: remove construction from PK only
    //  if (isToxId(id)) {
    //    toxId = QByteArray::fromHex(id.toLatin1());
    //  } else if (id.length() >= PUBLIC_KEY_HEX_CHARS) {
    //    toxId = QByteArray::fromHex(id.left(PUBLIC_KEY_HEX_CHARS).toLatin1());
    //  } else {
    //    toxId = QByteArray(); // invalid id string
    //  }
    if (isToxId(id)) {
        toxId = (id.toUtf8());
    }
}

/**
 * @brief Create a Ok ID from a QByteArray.
 *
 * If the given rawId is not a valid Ok ID, but can be a Public Key then:
 * publicKey == rawId and noSpam == 0 == checkSum.
 * If the given rawId isn't a valid Public Key or Ok ID a ToxId with all zero
 * bytes is created.
 *
 * @param rawId Ok ID bytes to convert to ToxId object
 */
ToxId::ToxId(const QByteArray& rawId) { constructToxId(rawId); }

/**
 * @brief Create a Ok ID from uint8_t bytes and lenght, convenience function
 * for toxcore interface.
 *
 * If the given rawId is not a valid Ok ID, but can be a Public Key then:
 * publicKey == rawId and noSpam == 0 == checkSum.
 * If the given rawId isn't a valid Public Key or Ok ID a ToxId with all zero
 * bytes is created.
 *
 * @param rawId Pointer to bytes to convert to ToxId object
 * @param len Number of bytes to read. Must be TOX_SECRET_KEY_SIZE for a Public
 * Key or TOX_ADDRESS_SIZE for a Ok ID.
 */
ToxId::ToxId(const uint8_t* rawId, int len) {
    QByteArray tmpId(reinterpret_cast<const char*>(rawId), len);
    constructToxId(tmpId);
}

void ToxId::constructToxId(const QByteArray& rawId) {
    // TODO: remove construction from PK only
    //  if (rawId.length() == TOX_SECRET_KEY_SIZE) {
    //    toxId = QByteArray(rawId); // construct from PK only
    //  } else if (rawId.length() == TOX_ADDRESS_SIZE &&
    //             isToxId(rawId.toHex().toUpper())) {
    //    toxId = QByteArray(rawId); // construct from full toxid
    //  } else {
    //    toxId = QByteArray(); // invalid id
    //  }
    toxId = QByteArray(rawId);
}

/**
 * @brief Compares the equality of the Public Key.
 * @param other Ok ID to compare.
 * @return True if both Tox IDs have the same public keys, false otherwise.
 */
bool ToxId::operator==(const ToxId& other) const { return toxId.compare(other.toxId) == 0; }

/**
 * @brief Compares the inequality of the Public Key.
 * @param other Ok ID to compare.
 * @return True if both Tox IDs have different public keys, false otherwise.
 */
bool ToxId::operator!=(const ToxId& other) const { return getPublicKey() != other.getPublicKey(); }

/**
 * @brief Returns the Ok ID converted to QString.
 * Is equal to getPublicKey() if the Ok ID was constructed from only a Public
 * Key.
 * @return The Ok ID as QString.
 */
QString ToxId::toString() const { return QString(toxId); }

/**
 * @brief Clears all elements of the Ok ID.
 */
void ToxId::clear() { toxId.clear(); }

/**
 * @brief Gets the ToxID as bytes, convenience function for toxcore interface.
 * @return The ToxID as uint8_t* if isValid() is true, else a nullptr.
 */
const uint8_t* ToxId::getBytes() const {
    if (isValid()) {
        return reinterpret_cast<const uint8_t*>(toxId.constData());
    }

    return nullptr;
}

/**
 * @brief Gets the Public Key part of the ToxID
 * @return Public Key of the ToxID
 */
FriendId ToxId::getPublicKey() const {
    //  auto const pkBytes = toxId.left(TOX_PUBLIC_KEY_SIZE);
    //  if (pkBytes.isEmpty()) {
    //    return ToxPk{};
    //  } else {
    //    return ToxPk{pkBytes};
    //  }
    return FriendId{toxId};
}

/**
 * @brief Returns the NoSpam value converted to QString.
 * @return The NoSpam value as QString or "" if the ToxId was constructed from a
 * Public Key.
 */
QString ToxId::getNoSpamString() const { return {}; }

/**
 * @brief Check, that id is a valid Ok ID.
 * @param id Ok ID to check.
 * @return True if id is a valid Ok ID, false otherwise.
 * @note Validates the checksum.
 */
bool ToxId::isValidToxId(const QString& id) { return isToxId(id) && ToxId(id).isValid(); }

/**
 * @brief Check, that id is probably a valid Ok ID.
 * @param id Ok ID to check.
 * @return True if the string can be a ToxID, false otherwise.
 * @note Doesn't validate checksum.
 */
bool ToxId::isToxId(const QString& id) {
    //    return id.length() == TOXID_HEX_CHARS && id.contains(ToxIdRegEx);
    return id.length() > 0;
}

/**
 * @brief Check it it's a valid Ok ID by verifying the checksum
 * @return True if it is a valid Ok ID, false otherwise.
 */
bool ToxId::isValid() const {
    if (toxId.length() > 0) {
        return true;
    }
    return false;
}

QString ToxId::getToxIdAsStr() const { return QString::fromUtf8(toxId); }
