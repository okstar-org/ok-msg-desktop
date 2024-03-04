#ifndef OPTIONACCESSOR_H
#define OPTIONACCESSOR_H

#include <QtPlugin>

class QString;

namespace ok{
namespace plugin{



class OptionAccessingHost;
class OptionAccessor {
public:
    virtual ~OptionAccessor() { }

    virtual void setOptionAccessingHost(OptionAccessingHost *host) = 0;

    virtual void optionChanged(const QString &option) = 0;
};
}
} // namespace ok

Q_DECLARE_INTERFACE(ok::plugin::OptionAccessor, "org.okstar.msg.OptionAccessor/0.1");

#endif // OPTIONACCESSOR_H
