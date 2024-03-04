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

#ifndef CONTACT_H
#define CONTACT_H

#include "src/core/contactid.h"
#include <QObject>
#include <QString>

class Contact : public QObject
{
    Q_OBJECT
public:
    virtual ~Contact() = 0;

    virtual void setName(const QString& name) = 0;
    virtual QString getDisplayedName() const = 0;
    virtual QString getId() const = 0;
    virtual const ContactId& getPersistentId() const = 0;
    virtual void setEventFlag(bool flag) = 0;
    virtual bool getEventFlag() const = 0;

signals:
    void displayedNameChanged(const QString& newName);
};

#endif // CONTACT_H
