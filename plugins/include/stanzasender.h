#ifndef STANZASENDER_H
#define STANZASENDER_H

#include <QtPlugin>

class StanzaSendingHost;

class StanzaSender {
public:
    virtual ~StanzaSender() { }

    virtual void setStanzaSendingHost(StanzaSendingHost *host) = 0;
};

Q_DECLARE_INTERFACE(StanzaSender, "org.okstar.msg.StanzaSender/0.1");

#endif // STANZASENDER_H
