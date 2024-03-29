#ifndef CONTACTSTATEACCESSOR_H
#define CONTACTSTATEACCESSOR_H

#include <QtPlugin>

class ContactStateAccessingHost;

class ContactStateAccessor {
public:
    virtual ~ContactStateAccessor() { }

    virtual void setContactStateAccessingHost(ContactStateAccessingHost *host) = 0;
};

Q_DECLARE_INTERFACE(ContactStateAccessor, "org.okstar.msg.ContactStateAccessor/0.2");

#endif // CONTACTSTATEACCESSOR_H
