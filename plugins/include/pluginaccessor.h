#ifndef PLUGINACCESSOR_H
#define PLUGINACCESSOR_H

#include <QtPlugin>

class PluginAccessingHost;

class PluginAccessor {
public:
    virtual ~PluginAccessor() { }

    virtual void setPluginAccessingHost(PluginAccessingHost *host) = 0;
};

Q_DECLARE_INTERFACE(PluginAccessor, "org.okstar.msg.PluginAccessor/0.1");

#endif // PLUGINACCESSOR_H
