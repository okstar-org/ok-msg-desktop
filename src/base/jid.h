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

#include <QHash>
#include <QScopedPointer>
#include <QString>

namespace ok::base {

static const QChar AT = '@';
static const QChar SLASH = '/';

class Jid {
public:
    Jid();
    ~Jid();

    explicit Jid(const char* s);
    explicit Jid(const QString& s);
    explicit Jid(const std::string& s);
    Jid(const QString& node, const QString& domain, const QString& resource = "");
    Jid& operator=(const QString& s);
    Jid& operator=(const char* s);
    Jid& operator=(const Jid& s);

    const QString& domain() const {
        return m_domain;
    }
    const QString& node() const {
        return m_node;
    }
    const QString& resource() const {
        return m_resource;
    }
    QString bare() const;
    QString full() const;

    bool isValid() const;
    bool isEmpty() const;

    bool compare(const Jid& a) const;

    inline bool operator==(const Jid& other) const {
        return compare(other);
    }
    inline bool operator!=(const Jid& other) const {
        return !(*this == other);
    }

private:
    void set(const QString& s);
    void set(const QString& domain, const QString& node, const QString& resource = "");

    void setDomain(const QString& s);
    void setNode(const QString& s);
    void setResource(const QString& resource);

private:
    void reset();

    QString m_domain, m_node, m_resource;
    bool valid = false;
};

Q_DECL_PURE_FUNCTION inline uint qHash(const Jid& key, uint seed = 0) Q_DECL_NOTHROW {
    return qHash(key.full(), seed);
}
}  // namespace ok::base
