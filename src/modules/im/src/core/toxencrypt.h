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

#ifndef TOXENCRYPT_H
#define TOXENCRYPT_H

#include <QByteArray>
#include <QString>

#include <memory>

struct Tox_Pass_Key;

class ToxEncrypt {
public:
    ~ToxEncrypt();
    ToxEncrypt() = delete;
    ToxEncrypt(const ToxEncrypt& other) = delete;
    ToxEncrypt& operator=(const ToxEncrypt& other) = delete;

    static int getMinBytes();
    static bool isEncrypted(const QByteArray& ciphertext);
    static QByteArray encryptPass(const QString& password, const QByteArray& plaintext);
    static QByteArray decryptPass(const QString& password, const QByteArray& ciphertext);
    static std::unique_ptr<ToxEncrypt> makeToxEncrypt(const QString& password);
    static std::unique_ptr<ToxEncrypt> makeToxEncrypt(const QString& password,
                                                      const QByteArray& toxSave);
    QByteArray encrypt(const QByteArray& plaintext) const;
    QByteArray decrypt(const QByteArray& ciphertext) const;

private:
    explicit ToxEncrypt(Tox_Pass_Key* key);

private:
    Tox_Pass_Key* passKey = nullptr;
};

#endif  // TOXENCRYPT_H
