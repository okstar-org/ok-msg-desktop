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

namespace ok::base {

//----------------------------------------------------------------------------
// Jid
//----------------------------------------------------------------------------
//

Jid::Jid() {
    valid = false;
}

Jid::~Jid() = default;

Jid::Jid(const std::string& s) {
    set(QString(s.c_str()));
}

Jid::Jid(const QString& s) {
    set(s);
}

Jid::Jid(const QString& node, const QString& domain, const QString& resource) {
    set(domain, node, resource);
}

Jid::Jid(const char* s) {
    set(QString(s));
}

Jid& Jid::operator=(const QString& s) {
    set(s);
    return *this;
}

Jid& Jid::operator=(const char* s) {
    set(QString(s));
    return *this;
}

Jid& Jid::operator=(const Jid& s) {
    m_domain = s.m_domain;
    m_node = s.m_node;
    m_resource = s.m_resource;
    return *this;
}

void Jid::reset() {
    m_domain = QString();
    m_node = QString();
    m_resource = QString();
    valid = false;
}

void Jid::set(const QString& s) {
    QString rest;
    int x = s.indexOf(SLASH);
    if (x != -1) {
        rest = s.mid(0, x);
        m_resource = s.mid(x + 1);
    } else {
        rest = s;
    }

    x = rest.indexOf(AT);
    if (x != -1) {
        m_node = rest.mid(0, x);
        m_domain = rest.mid(x + 1);
    } else {
        m_domain = rest;
    }

    valid = true;
}

void Jid::set(const QString& domain, const QString& node, const QString& resource) {
    m_node = node;
    m_domain = domain;
    m_resource = resource;
}

void Jid::setDomain(const QString& domain) {
    if (domain.isEmpty() || domain.contains(AT)) {
        return;
    }
    m_domain = domain;
}

void Jid::setNode(const QString& node) {
    if (node.isEmpty() || node.contains(AT)) return;
    m_node = node;
}

void Jid::setResource(const QString& resource) {
    if (resource.isEmpty() || resource.contains(SLASH)) return;
    m_resource = resource;
}

bool Jid::isValid() const {
    return valid;
}

bool Jid::isEmpty() const {
    return full().isEmpty();
}

QString Jid::bare() const {
    return m_node + AT + m_domain;
}

QString Jid::full() const {
    if (m_resource.isEmpty()) return bare();
    return bare() + SLASH + m_resource;
}

bool Jid::compare(const Jid& a) const {
    return full().compare(a.full());
}

}  // namespace ok::base
