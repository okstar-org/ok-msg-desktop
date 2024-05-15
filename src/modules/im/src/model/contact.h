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
    Contact();
    Contact(const ContactId& id, const QString& name, const QString& alias="", bool isGroup = false);
    virtual ~Contact() = 0;

    void setName(const QString& name);
    const QString& getName() const{return name;};

    void setAlias(const QString &name);
    const QString&  getAlias() const{return alias;};
    bool hasAlias() const { return !alias.isEmpty(); }


    QString getDisplayedName() const ;

    const ContactId& getPersistentId() const {return id;};
    QString getId() const {return id.toString(); };

    virtual void setEventFlag(bool flag) ;
    virtual bool getEventFlag() const ;

signals:
    void nameChanged(const QString &name);
    void displayedNameChanged(const QString& newName);
    void aliasChanged(QString alias);

protected:
    bool isGroup;
    ContactId id;

    //名称
    QString name;
    //别名(自己备注)
    QString alias;
};

#endif // CONTACT_H
