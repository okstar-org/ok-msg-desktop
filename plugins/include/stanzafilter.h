#ifndef STANZAFILTER_H
#define STANZAFILTER_H

#include <QtPlugin>

class QDomElement;

class StanzaFilter {
public:
    virtual ~StanzaFilter() { }

    // true = handled, don't pass to next handler

    virtual bool incomingStanza(int account, const QDomElement &xml) = 0;
    virtual bool outgoingStanza(int account, QDomElement &xml)       = 0;
};

Q_DECLARE_INTERFACE(StanzaFilter, "org.okstar.msg.StanzaFilter/0.1");

#endif // STANZAFILTER_H
