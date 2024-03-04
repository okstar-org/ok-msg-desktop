#ifndef ENCRYPTIONSUPPORT_H
#define ENCRYPTIONSUPPORT_H

#include <QtPlugin>

class QDomElement;

namespace ok {
namespace plugin {

class EncryptionSupport {
public:
  virtual ~EncryptionSupport() = default;

  // true = handled, don't pass to next handler
  virtual bool decryptMessageElement(int account, QDomElement &message) = 0;
  virtual bool encryptMessageElement(int account, QDomElement &message) = 0;
};
} // namespace plugin
} // namespace ok

Q_DECLARE_INTERFACE(ok::plugin::EncryptionSupport,
                    "org.okstar.msg.EncryptionSupport/0.1");

#endif // ENCRYPTIONSUPPORT_H
