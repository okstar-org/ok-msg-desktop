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

#include "toxencrypt.h"

#include <QByteArray>
#include <QDebug>
#include <QString>
#include <memory>

namespace module::im {

/**
 * @brief Frees the passKey before destruction.
 */
ToxEncrypt::~ToxEncrypt() {
    //    tox_pass_key_free(passKey);
}

/**
 * @brief Constructs a ToxEncrypt object from a Tox_Pass_Key.
 * @param key Derived key to use for encryption and decryption.
 */
ToxEncrypt::ToxEncrypt(Tox_Pass_Key* key) : passKey{key} {}

/**
 * @brief  Gets the minimum number of bytes needed for isEncrypted()
 * @return Minimum number of bytes needed to check if data was encrypted
 *         using this module.
 */
int ToxEncrypt::getMinBytes() {
    //    return TOX_PASS_ENCRYPTION_EXTRA_LENGTH;
    return 0;
}

/**
 * @brief Checks if the data was encrypted by this module.
 * @param ciphertext The data to check.
 * @return True if the data was encrypted using this module, false otherwise.
 */
bool ToxEncrypt::isEncrypted(const QByteArray& ciphertext) {
    //  if (ciphertext.length() < TOX_PASS_ENCRYPTION_EXTRA_LENGTH) {
    //    return false;
    //  }
    //
    //  return tox_is_data_encrypted(
    //      reinterpret_cast<const uint8_t *>(ciphertext.constData()));
    return false;
}

/**
 * @brief  Encrypts the plaintext with the given password.
 * @return Encrypted data or empty QByteArray on failure.
 * @param  password Password to encrypt the data.
 * @param  plaintext The data to encrypt.
 */
QByteArray ToxEncrypt::encryptPass(const QString& password, const QByteArray& plaintext) {
    //  if (password.length() == 0) {
    //    qWarning() << "Empty password supplied, probably not what you
    //    intended.";
    //  }
    //
    //  QByteArray pass = password.toUtf8();
    //  QByteArray ciphertext(plaintext.length() +
    //  TOX_PASS_ENCRYPTION_EXTRA_LENGTH,
    //                        0x00);
    //  TOX_ERR_ENCRYPTION error;
    //  tox_pass_encrypt(reinterpret_cast<const uint8_t *>(plaintext.constData()),
    //                   static_cast<size_t>(plaintext.size()),
    //                   reinterpret_cast<const uint8_t *>(pass.constData()),
    //                   static_cast<size_t>(pass.size()),
    //                   reinterpret_cast<uint8_t *>(ciphertext.data()), &error);
    //
    //  if (error != TOX_ERR_ENCRYPTION_OK) {
    //    qCritical() << getEncryptionError(error);
    //    return QByteArray{};
    //  }

    //  return ciphertext;
    return QByteArray{};
}

/**
 * @brief  Decrypts data encrypted with this module.
 * @return The plaintext or an empty QByteArray on failure.
 * @param  password The password used to encrypt the data.
 * @param  ciphertext The encrypted data.
 */
QByteArray ToxEncrypt::decryptPass(const QString& password, const QByteArray& ciphertext) {
    //  if (!isEncrypted(ciphertext)) {
    //    qWarning()
    //        << "The data was not encrypted using this module or it's
    //        corrupted.";
    //    return QByteArray{};
    //  }
    //
    //  if (password.length() == 0) {
    //    qDebug() << "Empty password supplied, probably not what you intended.";
    //  }
    //
    //  QByteArray pass = password.toUtf8();
    //  QByteArray plaintext(ciphertext.length() -
    //  TOX_PASS_ENCRYPTION_EXTRA_LENGTH,
    //                       0x00);
    //  TOX_ERR_DECRYPTION error;
    //  tox_pass_decrypt(reinterpret_cast<const uint8_t
    //  *>(ciphertext.constData()),
    //                   static_cast<size_t>(ciphertext.size()),
    //                   reinterpret_cast<const uint8_t *>(pass.constData()),
    //                   static_cast<size_t>(pass.size()),
    //                   reinterpret_cast<uint8_t *>(plaintext.data()), &error);
    //
    //  if (error != TOX_ERR_DECRYPTION_OK) {
    //    qWarning() << getDecryptionError(error);
    //    return QByteArray{};
    //  }

    //  return plaintext;
    return QByteArray{};
}

/**
 * @brief  Encrypts the plaintext with the stored key.
 * @return Encrypted data or empty QByteArray on failure.
 * @param  plaintext The data to encrypt.
 */
QByteArray ToxEncrypt::encrypt(const QByteArray& plaintext) const {
    if (!passKey) {
        //    qCritical() << "The passKey is invalid.";
        return QByteArray{};
    }
    return QByteArray{};
}

/**
 * @brief  Decrypts data encrypted with this module, using the stored key.
 * @return The plaintext or an empty QByteArray on failure.
 * @param  ciphertext The encrypted data.
 */
QByteArray ToxEncrypt::decrypt(const QByteArray& ciphertext) const {
    return ciphertext;
}
}  // namespace module::im