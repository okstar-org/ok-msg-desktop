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

#include "jid.h"
#include <QDebug>
namespace ok{
namespace base {

//----------------------------------------------------------------------------
// StringPrepCache
//----------------------------------------------------------------------------
QScopedPointer<StringPrepCache> StringPrepCache::_instance;

bool StringPrepCache::nameprep(const QString& in, int maxbytes, QString& out) {
    if (in.trimmed().isEmpty()) {
        out = QString();
        return false;  // empty names or just spaces are disallowed (rfc5892+rfc6122)
    }

    StringPrepCache* that = instance();

    auto it = that->nameprep_table.constFind(in);
    if (it != that->nameprep_table.constEnd()) {
        if (it.value().isNull()) {
            return false;
        }
        out = it.value();
        return true;
    }

    out = in;
    that->nameprep_table.insert(in, out);
    return true;
}

bool StringPrepCache::nodeprep(const QString& in, int maxbytes, QString& out) {
    if (in.isEmpty()) {
        out = QString();
        return true;
    }

    StringPrepCache* that = instance();

    auto it = that->nodeprep_table.constFind(in);
    if (it != that->nodeprep_table.constEnd()) {
        if (it.value().isNull()) {
            return false;
        }
        out = it.value();
        return true;
    }

    out = in;
#if 0
  if (stringprep(out, (Stringprep_profile_flags)0, stringprep_xmpp_nodeprep) !=
          0 ||
      out.size() > maxbytes) {
    that->nodeprep_table.insert(in, QString());
    return false;
  }
#endif
    that->nodeprep_table.insert(in, out);
    return true;
}

bool StringPrepCache::resourceprep(const QString& in, int maxbytes, QString& out) {
    if (in.isEmpty()) {
        out = QString();
        return true;
    }

    StringPrepCache* that = instance();

    auto it = that->resourceprep_table.constFind(in);
    if (it != that->resourceprep_table.constEnd()) {
        if (it.value().isNull()) {
            return false;
        }
        out = it.value();
        return true;
    }

    that->resourceprep_table.insert(in, out);
    return true;
}

bool StringPrepCache::saslprep(const QString& in, int maxbytes, QString& out) {
    if (in.isEmpty()) {
        out = QString();
        return true;
    }

    StringPrepCache* that = instance();

    auto it = that->saslprep_table.constFind(in);
    if (it != that->saslprep_table.constEnd()) {
        if (it.value().isNull()) {
            return false;
        }
        out = it.value();
        return true;
    }

    out = in;
#if 0
  if (stringprep(out, (Stringprep_profile_flags)0, stringprep_saslprep) != 0 ||
      out.size() > maxbytes) {
    that->saslprep_table.insert(in, QString());
    return false;
  }
#endif
    that->saslprep_table.insert(in, out);
    return true;
}

void StringPrepCache::cleanup() { _instance.reset(nullptr); }

StringPrepCache* StringPrepCache::instance() {
    if (!_instance) {
        _instance.reset(new StringPrepCache);
    }
    return _instance.data();
}

StringPrepCache::StringPrepCache() {}

//----------------------------------------------------------------------------
// Jid
//----------------------------------------------------------------------------
//
static inline bool validDomain(const QString& s, QString& norm) {
    return StringPrepCache::nameprep(s, 1024, norm);
}

static inline bool validNode(const QString& s, QString& norm) {
    return StringPrepCache::nodeprep(s, 1024, norm);
}

static inline bool validResource(const QString& s, QString& norm) {
    return StringPrepCache::resourceprep(s, 1024, norm);
}

Jid::Jid() {
    valid = false;
    null = true;
}

Jid::~Jid() {}

Jid::Jid(const QString& s) { set(s); }

Jid::Jid(const QString& node, const QString& domain, const QString& resource) {
    set(domain, node, resource);
}

Jid::Jid(const char* s) { set(QString(s)); }

Jid& Jid::operator=(const QString& s) {
    set(s);
    return *this;
}

Jid& Jid::operator=(const char* s) {
    set(QString(s));
    return *this;
}

Jid& Jid::operator=(const Jid& s) {
    f = s.f;
    b = s.b;
    d = s.d;
    n = s.n;
    r = s.r;
    return *this;
}

void Jid::reset() {
    f = QString();
    b = QString();
    d = QString();
    n = QString();
    r = QString();
    valid = false;
    null = true;
}

void Jid::update() {
    // build 'bare' and 'full' jids
    if (n.isEmpty())
        b = d;
    else
        b = n + '@' + d;
    if (r.isEmpty())
        f = b;
    else
        f = b + '/' + r;
    if (f.isEmpty()) valid = false;
    null = f.isEmpty() && r.isEmpty();
}

void Jid::set(const QString& s) {
    QString rest, domain, node, resource;
    QString norm_domain, norm_node, norm_resource;
    int x = s.indexOf('/');
    if (x != -1) {
        rest = s.mid(0, x);
        resource = s.mid(x + 1);
    } else {
        rest = s;
        resource = QString();
    }
    //  if (!validResource(resource, norm_resource)) {
    //    qDebug()<<"resource"<<resource;
    //    reset();
    //    return;
    //  }

    x = rest.indexOf('@');
    if (x != -1) {
        node = rest.mid(0, x);
        domain = rest.mid(x + 1);
    } else {
        node = QString();
        domain = rest;
    }
    if (!validDomain(domain, norm_domain) || !validNode(node, norm_node)) {
        reset();
        return;
    }

    valid = true;
    null = false;
    d = norm_domain;
    n = norm_node;
    r = norm_resource;
    update();
}

void Jid::set(const QString& domain, const QString& node, const QString& resource) {
    QString norm_domain, norm_node, norm_resource;
    if (!validDomain(domain, norm_domain) || !validNode(node, norm_node) ||
        !validResource(resource, norm_resource)) {
        reset();
        return;
    }
    valid = true;
    null = false;
    d = norm_domain;
    n = norm_node;
    r = norm_resource;
    update();
}

void Jid::setDomain(const QString& s) {
    if (!valid) return;
    QString norm;
    if (!validDomain(s, norm)) {
        reset();
        return;
    }
    d = norm;
    update();
}

void Jid::setNode(const QString& s) {
    if (!valid) return;
    QString norm;
    if (!validNode(s, norm)) {
        reset();
        return;
    }
    n = norm;
    update();
}

void Jid::setResource(const QString& s) {
    if (!valid) return;
    QString norm;
    if (!validResource(s, norm)) {
        reset();
        return;
    }
    r = norm;
    update();
}

Jid Jid::withNode(const QString& s) const {
    Jid j = *this;
    j.setNode(s);
    return j;
}

Jid Jid::withDomain(const QString& s) const {
    Jid j = *this;
    j.setDomain(s);
    return j;
}

Jid Jid::withResource(const QString& s) const {
    Jid j = *this;
    j.setResource(s);
    return j;
}

bool Jid::isValid() const { return valid; }

bool Jid::isEmpty() const { return f.isEmpty(); }

bool Jid::compare(const Jid& a, bool compareRes) const {
    if (null && a.null) return true;

    // only compare valid jids
    if (!valid || !a.valid) return false;

    return !(compareRes ? (f != a.f) : (b != a.b));
}
}  // namespace base
}
