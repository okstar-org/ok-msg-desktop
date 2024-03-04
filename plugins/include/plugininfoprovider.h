#ifndef PLUGININFOPROVIDER_H
#define PLUGININFOPROVIDER_H

#include <QtPlugin>

class QString;

namespace ok {
namespace plugin {

class PluginInfoProvider {
public:
  virtual ~PluginInfoProvider() {}

  virtual QString pluginInfo() = 0;
};

} // namespace plugin
} // namespace ok

Q_DECLARE_INTERFACE(ok::plugin::PluginInfoProvider,
                    "org.okstar.msg.PluginInfoProvider/0.1");

#endif // PLUGININFOPROVIDER_H
