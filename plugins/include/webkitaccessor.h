#ifndef WEBKITACCESSOR_H
#define WEBKITACCESSOR_H

#include <QtPlugin>
namespace ok{
namespace plugin{


class WebkitAccessingHost;

class WebkitAccessor {
public:
    virtual ~WebkitAccessor() { }

    virtual void setWebkitAccessingHost(WebkitAccessingHost *host) = 0;
};
}
}
Q_DECLARE_INTERFACE(ok::plugin::WebkitAccessor, "org.okstar.msg.WebkitAccessor/0.1");

#endif // WEBKITACCESSOR_H
