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

#ifndef XMPP_JID_H
#define XMPP_JID_H

#include <QByteArray>
#include <QHash>
#include <QScopedPointer>
#include <QString>
namespace ok::base {
class StringPrepCache {
public:
    static bool nameprep(const QString& in, int maxbytes, QString& out);
    static bool nodeprep(const QString& in, int maxbytes, QString& out);
    static bool resourceprep(const QString& in, int maxbytes, QString& out);
    static bool saslprep(const QString& in, int maxbytes, QString& out);

    static void cleanup();

private:
    QHash<QString, QString> nameprep_table;
    QHash<QString, QString> nodeprep_table;
    QHash<QString, QString> resourceprep_table;
    QHash<QString, QString> saslprep_table;

    static QScopedPointer<StringPrepCache> _instance;
    static StringPrepCache* instance();

    StringPrepCache();
};

class Jid : public QChar {
public:
    Jid();
    ~Jid();

    Jid(const QString& s);
    Jid(const QString& node, const QString& domain, const QString& resource = "");
    Jid(const char* s);
    Jid& operator=(const QString& s);
    Jid& operator=(const char* s);
    Jid& operator=(const Jid& s);

    bool isNull() const { return null; }
    const QString& domain() const { return d; }
    const QString& node() const { return n; }
    const QString& resource() const { return r; }
    const QString& bare() const { return b; }
    const QString& full() const { return f; }

    Jid withNode(const QString& s) const;
    Jid withDomain(const QString& s) const;
    Jid withResource(const QString& s) const;

    bool isValid() const;
    bool isEmpty() const;
    bool compare(const Jid& a, bool compareRes = true) const;
    inline bool operator==(const Jid& other) const { return compare(other, true); }
    inline bool operator!=(const Jid& other) const { return !(*this == other); }

private:
    void set(const QString& s);
    void set(const QString& domain, const QString& node, const QString& resource = "");

    void setDomain(const QString& s);
    void setNode(const QString& s);
    void setResource(const QString& s);

private:
    void reset();
    void update();

    QString f, b, d, n, r;
    bool valid, null;
};

Q_DECL_PURE_FUNCTION inline uint qHash(const Jid& key, uint seed = 0) Q_DECL_NOTHROW {
    return qHash(key.full(), seed);
}
}

#endif  // XMPP_JID_H
