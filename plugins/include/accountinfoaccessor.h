#ifndef ACCOUNTINFOACCESSOR_H
#define ACCOUNTINFOACCESSOR_H

#include <QtPlugin>

class AccountInfoAccessingHost;

class AccountInfoAccessor {
public:
    virtual ~AccountInfoAccessor() { }

    virtual void setAccountInfoAccessingHost(AccountInfoAccessingHost *host) = 0;
};

Q_DECLARE_INTERFACE(AccountInfoAccessor, "org.okstar.msg.AccountInfoAccessor/0.1");

#endif // ACCOUNTINFOACCESSOR_H
