#ifndef APPLICATIONINFOACCESSOR_H
#define APPLICATIONINFOACCESSOR_H

#include <QtPlugin>

class ApplicationInfoAccessingHost;

class ApplicationInfoAccessor {
public:
    virtual ~ApplicationInfoAccessor() { }

    virtual void setApplicationInfoAccessingHost(ApplicationInfoAccessingHost *host) = 0;
};

Q_DECLARE_INTERFACE(ApplicationInfoAccessor, "org.okstar.msg.ApplicationInfoAccessor/0.1");

#endif // APPLICATIONINFOACCESSOR_H
