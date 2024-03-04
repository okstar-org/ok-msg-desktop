/*
 * Copyright (c) 2022 船山信息 chuanshaninfo.com
 * OkEDU is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PubL v2 for more details.
 */

#pragma once

#include <QtCore>
#include <QtPlugin>

class QWidget;

namespace ok {
namespace plugin{


// see PluginHost::selfMetadata for possible properties
#define PSI_PLUGIN_MD(prop) property("metadata").toMap().value(QLatin1String(prop))

/**
 * \brief An abstract class for implementing a plugin
 */
class OkPlugin {
//  Q_OBJECT
public:
    // Priorities allows plugins to make processing more ordered. For example
    // some plugins may require process stanzas as early as possible, others
    // may want to do some work at the end. So here are 5 levels of
    // priority which plugin may choose from. If plugin is not aware about
    // priority then Normal will be choosed for it.
    // While writing plugins its desirable to think twice before choosing
    // Lowest or Highest priority, since your plugin may be not the only which
    // need it. Think about for example stopspam plugin which is known to be
    // highest prioroty blocker/processor. Are you writing stopspam? If not
    // choose High if you want something more then Normal.
    enum Priority {
        PriorityLowest  = 0, // always in the end. last loaded Lowest plugin moves other Lowest to Low side
        PriorityLow     = 1,
        PriorityNormal  = 2, // default
        PriorityHigh    = 3,
        PriorityHighest = 4, // always in the start. last loaded Highest plugin moves others to High side
    };

    virtual ~OkPlugin() { }

    /**
     * \brief Plugin Name
     * The full name of the plugin.
     * \return Plugin name
     */
    virtual QString name() const = 0;

    /**
     * \brief Plugin options widget
     * This method is called by the Psi options system to retrieve
     * a widget containing the options for this plugin.
     * This will then be embedded in the options dialog, so this
     * should be considered when designing the widget. Should return
     * nullptr when there are no user-configurable options. The calling method
     * is responsible for deleting the options.
     *
     * TODO: make sure this is really deleted, etc
     */
    virtual QWidget *options() = 0;

    /**
     * \brief Enable plugin
     * \return true if plugin was successfully enabled
     */

    virtual bool enable() = 0;

    /**
     * \brief Disable plugin
     * \return true if plugin was successfully disabled
     */
    virtual bool disable() = 0;

    virtual void applyOptions()   = 0;
    virtual void restoreOptions() = 0;

    virtual QStringList pluginFeatures() { return QStringList(); }
};

}
}

Q_DECLARE_INTERFACE(ok::plugin::OkPlugin, "org.okstar.msg.OkPlugin/0.1");
