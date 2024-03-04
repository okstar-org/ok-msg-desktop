#ifndef PSIMEDIAACCESSOR_H
#define PSIMEDIAACCESSOR_H

#include <QtPlugin>
namespace ok {
namespace plugin {

class PsiMediaHost;

class PsiMediaAccessor {
public:
  virtual ~PsiMediaAccessor() {}

  virtual void setPsiMediaHost(PsiMediaHost *host) = 0;
};
} // namespace plugin
} // namespace ok

Q_DECLARE_INTERFACE(ok::plugin::PsiMediaAccessor,
                    "org.okstar.msg.PsiMediaAccessor/0.1");

#endif // PSIMEDIAACCESSOR_H
