/*
 * Copyright (c) 2022 船山信息 chuanshaninfo.com
 * The project is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PubL v2 for more details.
 */

#include "pluginmanager.h"

#include <QLabel>
#include <QMetaObject>
#include <QObject>
#include <QPluginLoader>
#include <QQueue>
#include <QtCore>
#include "OkOptions.h"
#include "base/OkAccount.h"
#include "base/OkSettings.h"
#include "base/basic_types.h"
#include "base/task.h"
#include "files.h"
#include "iqnamespacefilter.h"
#include "lib/messenger/messenger.h"
#include "lib/settings/applicationinfo.h"
#include "okplugin.h"
#include "pluginhost.h"
/**
 * Helper class used to process incoming XML in plugins.
 * This task should work as long as the related account exists,
 * that's why we override onDisconnect (called when XML stream ends)
 * to prevent it from stopping prematurely.
 *
 * According to common sense, tasks should have at least
 * a vaguely defined execution time, however, this one runs
 * indefinitely long and feels more like a hook/handler.
 * Therefore it should probably be refactored to one.
 */

namespace ok {
namespace plugin {

class PluginManager::StreamWatcher : public QObject {
public:
    StreamWatcher(::lib::messenger::Messenger* messenger, PluginManager* pluginManager,
                  int account_)  //
            : manager(pluginManager), account(account_), m_messenger(messenger) {
        // ignore
        connect(m_messenger, &::lib::messenger::Messenger::incoming,  //
                this, &StreamWatcher::doDom, Qt::QueuedConnection);
    }

    ~StreamWatcher() {}

    PluginManager* manager;
    ::lib::messenger::Messenger* m_messenger;
    int account;

public slots:
    void doDom(QString xml) {
        //    qDebug() << "incomingXml=>" << xml;
        QDomDocument document;
        if (!document.setContent(xml, true)) {
            return;
        }
        auto ele = document.documentElement();
        manager->incomingXml(account, ele);
        //    bool y =
        //    qDebug() << "incomingXml=>" << y;
    };
};

/**
 * Function to obtain all the directories in which plugins can be stored
 * \return List of plugin directories
 */
QStringList PluginManager::pluginDirs() { return ApplicationInfo::pluginDirs(); }

/**
 * Method for accessing the singleton instance of the class.
 * Instanciates if no instance yet exists.
 * \return Pointer to the plugin manager.
 */
PluginManager* PluginManager::instance() {
    if (!instance_) {
        instance_ = new PluginManager();
    }
    return instance_;
}

/**
 * Default constructor. Locates all plugins, sets watchers on those directories
 * to locate new ones and loads those enabled in the config.
 */
PluginManager::PluginManager()
        : QObject(nullptr)
        ,  //
        psi_(nullptr)
        ,  //
        m_thread(std::make_unique<QThread>()) {
    updatePluginsList();
    if (!m_thread) {
        qCritical() << "could not allocate PluginManager thread";
        return;
    }
    m_thread->setObjectName("PluginManager");
    this->moveToThread(m_thread.get());
    m_thread->start();

    auto const& dirs = pluginDirs();
    for (const QString& path : dirs) {
        qDebug() << "path " << path;
        //        QCA::DirWatch *dw = new QCA::DirWatch(path, this);
        //        connect(dw, SIGNAL(changed()), SLOT(dirsChanged()));
        //        dirWatchers_.append(dw);
    }

    _messageViewJSFiltersTimer = new QTimer(this);
    _messageViewJSFiltersTimer->setSingleShot(true);
    _messageViewJSFiltersTimer->setInterval(10);
    // to be able to restart in case of batch events
    connect(_messageViewJSFiltersTimer, &QTimer::timeout, this, &PluginManager::jsFiltersUpdated);

    //    connect(PsiOptions::instance(), &PsiOptions::optionChanged, this,
    //    &PluginManager::optionChanged);
}

void PluginManager::initNewSession(PsiCon* psi) {
    psi_ = psi;
    clients_.clear();
    accountIds_.clear();
    loadEnabledPlugins();
}

/**
 * Updates list of known plugins by reading all plugin directories
 * and returns the list of new plugins
 */
QList<PluginHost*> PluginManager::updatePluginsList() {
    QList<PluginHost*> newPlugins;

    auto const& dirs = pluginDirs();
    for (const QString& d : dirs) {
        QDir dir(d);
        auto const& entries = dir.entryInfoList(QDir::Files);
        for (const QFileInfo& fileInfo : entries) {
            QString file = fileInfo.canonicalFilePath();
            auto pp = addHostFile(file);
            if (pp) {
                newPlugins.append(pp);
            }
        }
    }

    return newPlugins;
}

bool PluginManager::uninstallPlugin(const QString& shortName) {
    for (const auto& plugin : availablePlugins()) {
        auto host = this->plugin(plugin);
        if (host) {
            if (host->shortName() == shortName) {
                qDebug() << "Uninstall plugin:" << shortName;
                if (host->unload()) {
                    removeHostFile(host);
                    host->deleteLater();
                    return true;
                };
            }
        }
    }
    return false;
}

bool PluginManager::installPlugin(const QString& filePath, const QString& shortName) {
    // 移动到插件目录
    qDebug() << "Plugin is:" << filePath << shortName;
    QFile file(filePath);
    if (!file.exists()) {
        qWarning() << "Plugin is not exist!";
        return false;
    }

    auto pluginFile = ok::base::OkSettings().getAppPluginPath().path() + '/' + shortName;
    auto moved = ok::base::Files::moveFile(filePath, pluginFile);
    if (!moved) {
        qWarning() << "Plugin move failed!";
        return false;
    }

    qDebug() << "Plugin at:" << pluginFile;
    auto ph = addHostFile(pluginFile);
    if (ph == nullptr) {
        return false;
    }
    return ph->load();
}

PluginHost* PluginManager::addHostFile(const QString& file) {
    qDebug() << "addHostFile:" << file;
    if (!QLibrary::isLibrary(file)) {
        qDebug("Is not a library: %s", qPrintable(file));
        return nullptr;
    }
    qDebug("Found plugin: %s", qPrintable(file));
    //  if (pluginByFile_.contains(file)) {
    //    qDebug() << "The plugin:" << file << "is be contained.";
    //    return nullptr;
    //  }

    auto* host = new PluginHost(this, file);
    connect(host, &PluginHost::enabled, this,
            [this, shortName = host->shortName()]() { emit pluginEnabled(shortName); });
    connect(host, &PluginHost::disabled, this,
            [this, shortName = host->shortName()]() { emit pluginDisabled(shortName); });
    if (!host->isValid()) {
        qDebug() << "The plugin is invalid.";
        return nullptr;
    }

    if (host->shortName().isEmpty()) {
        qWarning() << "The plugin:" << host->name() << " is empty.";
        return nullptr;
    }

    if (hosts_.contains(host->shortName())) {
        qWarning() << "The plugin:" << host->name() << "is loaded.";
        return nullptr;
    }

    hosts_[host->shortName()] = host;
    pluginByFile_[file] = host;

    if (host->priority() == OkPlugin::PriorityHighest || !pluginsByPriority_.size()) {
        pluginsByPriority_.push_front(host);
    } else {
        // find position for insertion
        int i = pluginsByPriority_.size();
        while (i && host->priority() > pluginsByPriority_[i - 1]->priority()) {
            i--;
        }
        pluginsByPriority_.insert(i, host);
    }

    host->enable();
    return host;
}

void PluginManager::removeHostFile(PluginHost* host) {
    if (!host) {
        qWarning() << "Plugin is not installed.";
        return;
    }

    if (host->isEnabled()) {
        qWarning() << "Plugin is not disabled.";
        return;
    }

    auto file = pathToPlugin(host->shortName());
    qDebug() << "Plugin file:" << file;

    int host_removed = hosts_.remove(host->shortName());
    qDebug() << "Remove plugin from host:" << host->shortName() << "=>" << host_removed;

    int file_removed2 = pluginByFile_.remove(file);
    qDebug() << "Remove plugin from file:" << file << "=>" << file_removed2;

    pluginsByPriority_.removeAll(host);
    qDebug() << "Remove plugin from pluginsByPriority";

    auto removed = ok::base::Files::removeFile(file);
    qDebug() << "Remove plugin local file:" << file << "=>" << removed;
}

/**
 * This slot is executed when the contents of a plugin directory changes
 * It causes the available plugin list to be refreshed.
 */
void PluginManager::dirsChanged() {
    auto const& pl = updatePluginsList();
    for (PluginHost* plugin : pl) {
        loadPluginIfEnabled(plugin);
    }
}

void PluginManager::accountDestroyed() {
    auto* pa = static_cast<ok::base::OkAccount*>(sender());
    accountIds_.removeAccount(pa);
}

/**
 * This causes all plugins that are both set for auto-load, and available
 * to be loaded.
 */
void PluginManager::loadEnabledPlugins() {
#ifndef PLUGINS_NO_DEBUG
    qDebug("Loading enabled plugins");
#endif
    for (PluginHost* plugin : qAsConst(pluginsByPriority_)) {
        loadPluginIfEnabled(plugin);
    }
}

void PluginManager::loadPluginIfEnabled(PluginHost* host) {
    //    const QString option = QString("%1.%2").arg(loadOptionPrefix,
    //    plugin->shortName()); QVariant      load   =
    //    PsiOptions::instance()->getOption(option, false); if (load.toBool()) {
    // #ifndef PLUGINS_NO_DEBUG
    //        qDebug("Plugin %s is enabled in config: loading",
    //        qPrintable(plugin->shortName()));
    // #endif
    //        plugin->enable();
    //    }
}

/**
 * Called when an option changes to load or unload a plugin if it's a plugin
 * option
 * \param option Option changed
 */
void PluginManager::optionChanged(const QString& option) {
    bool pluginOpt = option.startsWith(loadOptionPrefix);
    const QString shortName = option.split(".").last();
    for (PluginHost* plugin : qAsConst(pluginByFile_)) {
        plugin->optionChanged(option);
        if (pluginOpt && plugin->shortName() == shortName) {
            bool shouldUpdateFeatures;
            if (OkOptions::instance()->getOption(option).toBool()) {
                plugin->enable();
                shouldUpdateFeatures = !plugin->pluginFeatures().isEmpty();
            } else {
                shouldUpdateFeatures = !plugin->pluginFeatures().isEmpty();
                if (optionsWidget_) delete optionsWidget_;
                plugin->unload();
            }
            if (shouldUpdateFeatures) {
                updateFeatures();
            }
        }
    }
}

void PluginManager::updateFeatures() {
    //  for (int acc_id = 0; accountIds_.isValidRange(acc_id); ++acc_id) {
    //    OkAccount *pa = accountIds_.account(acc_id);
    //    pa->updateFeatures();
    //        if (pa->isConnected()) {
    //            pa->setStatusActual(pa->status());
    //        }
    //  }
}

/**
 * Loads all available plugins
 */
void PluginManager::loadAllPlugins() {
    qDebug("Loading all plugins");
    // Now look for external plugins
    for (PluginHost* plugin : qAsConst(hosts_)) {
        plugin->load();
        if (plugin->enable()) emit pluginEnabled(plugin->shortName());
    }
}

/**
 * Unloads all Psi plugins.
 * \return Success of unloading all plugins; if any plugins couldn't be
 *         unloaded, false.
 */
bool PluginManager::unloadAllPlugins() {
#ifndef PLUGINS_NO_DEBUG
    qDebug("Unloading all plugins");
#endif
    bool ok = true;
    for (PluginHost* plugin : qAsConst(hosts_)) {
        if (!plugin->unload()) {
            ok = false;
        }
    }
    return ok;
}

bool PluginManager::isAvailable(const QString& plugin) const { return hosts_.contains(plugin); }

bool PluginManager::isEnabled(const QString& plugin) const {
    bool enabled = false;
    if (hosts_.contains(plugin)) {
        enabled = hosts_[plugin]->isEnabled();
    }
    return enabled;
}

/**
 * Find the file which provides the named plugin. If the named plugin is not
 * known, an empty string is provided.
 * \param plugin Name of the plugin.
 * \return Path to the plugin file.
 */
QString PluginManager::pathToPlugin(const QString& plugin) const {
    QString path;
    if (hosts_.contains(plugin)) {
        path = hosts_[plugin]->path();
    }
    return path;
}

/**
 * Returns short name of the named plugin. If the named plugin is not
 * known, an empty string is provided.
 * \param plugin Name of the plugin.
 * \return Path to the plugin file.
 */
QString PluginManager::shortName(const QString& plugin) const {
    QString name;
    if (hosts_.contains(plugin)) {
        name = hosts_[plugin]->shortName();
    }
    return name;
}

QString PluginManager::pluginName(const QString& shortName) const {
    auto host = hosts_.value(shortName);
    return host ? host->name() : QString();
}

QString PluginManager::version(const QString& plugin) const {
    QString name;
    if (hosts_.contains(plugin)) {
        name = hosts_[plugin]->version();
    }
    return name;
}

QString PluginManager::vendor(const QString& plugin) const {
    auto it = hosts_.find(plugin);
    return it == hosts_.end() ? QString() : it.value()->vendor();
}

QString PluginManager::description(const QString& plugin) const {
    auto it = hosts_.find(plugin);
    return it == hosts_.end() ? QString() : it.value()->description();
}

/**
 * Returns a list of available plugin names found in all plugin directories.
 */
QStringList PluginManager::availablePlugins() const { return hosts_.keys(); }

/**
 * Provides a pointer to a QWidget providing the options dialog for the
 * named plugin, if it exists, else nullptr.
 * \param plugin Name of the plugin.
 * \return Pointer to the options widget for the named plugin.
 */
QWidget* PluginManager::optionsWidget(const QString& plugin) {
    if (optionsWidget_) {
        delete optionsWidget_;
    }

    if (hosts_.contains(plugin)) {
        optionsWidget_ = hosts_[plugin]->optionsWidget();
    }

    if (!optionsWidget_) {
#ifndef PLUGINS_NO_DEBUG
        qWarning("Attempting to get options for %s which doesn't exist", qPrintable(plugin));
#endif
        optionsWidget_ = new QLabel(tr("This plugin has no user configurable options"));
    }
    return optionsWidget_;
}

/**
 * \brief Give each plugin the opportunity to set shortcuts
 *
 * Each plugin can set its own global shortcuts
 */
void PluginManager::setShortcuts() {
    for (PluginHost* host : qAsConst(pluginByFile_)) {
        host->setShortcuts();
    }
}

/**
 * \brief Give each plugin the opportunity to process the incoming message event
 *
 * Each plugin is passed the event in turn. Any plugin may then modify the event
 * and may cause the event to be silently discarded.
 *
 * \param account Pointer to the OkAccount responsible
 * \param event Incoming event
 * \return Continue processing the event; true if the event should be silently
 * discarded.
 */
bool PluginManager::processMessage(OkAccount* account, const QString& jidFrom, const QString& body,
                                   const QString& subject) {
    bool handled = false;
    for (PluginHost* host : qAsConst(pluginsByPriority_)) {
        if (host->processMessage(accountIds_.id(account), jidFrom, body, subject)) {
            handled = true;
            break;
        }
    }
    return handled;
}

/**
 * \brief Give each plugin the opportunity to process the incoming event
 *
 * Each plugin is passed the event in turn. Any plugin may then modify the event
 * and may cause the event to be silently discarded.
 *
 * \param account Pointer to the OkAccount responsible
 * \param event Incoming event
 * \return Continue processing the event; true if the event should be silently
 * discarded.
 */
bool PluginManager::processEvent(OkAccount* account, QDomElement& event) {
    bool handled = false;
    const int acc_id = accountIds_.id(account);
    for (PluginHost* host : qAsConst(pluginsByPriority_)) {
        if (host->processEvent(acc_id, event)) {
            handled = true;
            break;
        }
    }
    return handled;
}

/**
 * process an outgoing message
 */
bool PluginManager::processOutgoingMessage(OkAccount* account, const QString& jidTo, QString& body,
                                           const QString& type, QString& subject) {
    bool handled = false;
    const int acc_id = accountIds_.id(account);
    for (PluginHost* host : qAsConst(pluginByFile_)) {
        if (host->processOutgoingMessage(acc_id, jidTo, body, type, subject)) {
            handled = true;
            break;
        }
    }
    return handled;
}

void PluginManager::processOutgoingStanza(OkAccount* account, QDomElement& stanza) {
    const int acc_id = accountIds_.id(account);
    for (PluginHost* host : qAsConst(pluginByFile_)) {
        if (host->outgoingXml(acc_id, stanza)) {
            break;
        }
    }
}

/**
 * Notify to plugins that an account is going to connect.
 */
void PluginManager::startLogin(OkAccount* account) {
    const int acc_id = accountIds_.id(account);
    for (PluginHost* host : qAsConst(pluginByFile_)) {
        host->logout(acc_id);
        emit accountBeforeLogin(acc_id);
    }
}

/**
 * Notify to plugins that an account will go offline now.
 */
void PluginManager::logout(OkAccount* account) {
    const int acc_id = accountIds_.id(account);
    for (PluginHost* host : qAsConst(pluginByFile_)) {
        host->logout(acc_id);
        emit accountLoggedOut(acc_id);
    }
}

void PluginManager::addSettingPage(OAH_PluginOptionsTab* tab) {
    settingsTabs_.append(tab);
    //    OptionsDlg *w = qobject_cast<OptionsDlg
    //    *>(psi_->dialogFind("OptionsDlg")); if (w) {
    //        w->addPluginWrapperTab(tab);
    //    }
}

void PluginManager::removeSettingPage(OAH_PluginOptionsTab* tab) {
    //  OptionsDlg *w = qobject_cast<OptionsDlg
    //  *>(psi_->dialogFind("OptionsDlg")); if (w) {
    //    w->removeTab(tab->id());
    //  }
    //  settingsTabs_.removeOne(tab);
}

QList<OAH_PluginOptionsTab*> PluginManager::settingsPages() const { return settingsTabs_; }

/**
 * \brief Give each plugin the opportunity to process the incoming xml
 *
 * Each plugin is passed the xml in turn using various filter interfaces
 * (for example, see StanzaFilter or IqFilter).
 * Any plugin may then modify the xml and may cause the stanza to be
 * silently discarded.
 *
 * \param account Identifier of the OkAccount responsible
 * \param xml Incoming XML
 * \return Continue processing the event; true if the event should be silently
 * discarded.
 */
bool PluginManager::incomingXml(int account, const QDomElement& xml) {
    bool handled = false;
    for (PluginHost* host : qAsConst(pluginsByPriority_)) {
        if (host->incomingXml(account, xml)) {
            handled = true;
            break;
        }
    }
    return handled;
}

/**
 * Called by PluginHost when its hosted plugin wants to send xml stanza.
 *
 * \param account Identifier of the OkAccount responsible
 * \param xml XML stanza to be sent
 */
void PluginManager::sendXml(int account, const QString& xml) {
    if (account < 0) {
        qWarning() << "account is" << account << " that is invalid";
        return;
    }

    if (account < clients_.size()) {
        clients_[account]->send(xml);
    }
}

/**
 * Returns unique stanza identifier in account's stream
 *
 * \param account Identifier of the OkAccount responsible
 * \return Unique ID to be used for when sending a stanza
 */
QString PluginManager::uniqueId(int account) const {
    QString id;
    if (account < clients_.size()) {
        id = clients_[account]->genUniqueId();
    }
    return id;
}

QString PluginManager::getStatus(int account) const {
    //  Status S;
    //  OkAccount *pa = accountIds_.account(account);
    //  if (pa)
    //    S = pa->status();
    //  return S.typeString();
    return {};
}

QString PluginManager::getStatusMessage(int account) const {
    //  Status S;
    //  OkAccount *pa = accountIds_.account(account);
    //  if (pa)
    //    S = pa->status();
    //  return S.status();
    return {};
}

static inline const QString getProxyId(OkAccount* pa) {
    //  return pa->accountOptions().proxyID;
    return {};
}

QString PluginManager::proxyHost(int account) const {
    QString host;
    //  OkAccount *pa = accountIds_.account(account);
    //  if (pa)
    //    host = ProxyManager::instance()->getItem(getProxyId(pa)).settings.host;
    return host;
}

int PluginManager::proxyPort(int account) const {
    int port = -1;
    //  OkAccount *pa = accountIds_.account(account);
    //  if (pa)
    //    port = ProxyManager::instance()->getItem(getProxyId(pa)).settings.port;
    return port;
}

QString PluginManager::proxyUser(int account) const {
    QString user;
    //  OkAccount *pa = accountIds_.account(account);
    //  if (pa)
    //    user = ProxyManager::instance()->getItem(getProxyId(pa)).settings.user;
    return user;
}

QString PluginManager::proxyPassword(int account) const {
    QString pass;
    //  OkAccount *pa = accountIds_.account(account);
    //  if (pa)
    //    pass = ProxyManager::instance()->getItem(getProxyId(pa)).settings.pass;
    return pass;
}

QStringList PluginManager::getRoster(int account) const {
    QStringList list;
    //  list << "-1";
    //  if (accountIds_.isValidRange(account)) {
    //    list.clear();
    //    OkAccount *pa = accountIds_.account(account);
    //    if (pa) {
    //      QList<PsiContact *> roster = pa->contactList();
    //      for (int i = 0; i < roster.size(); i++) {
    //        list.push_back(roster.at(i)->jid().bare());
    //      }
    //    }
    //  }
    return list;
}

QString PluginManager::getJid(int account) const {
    QString jid = "";
    if (accountIds_.isValidRange(account)) {
        jid.clear();
        OkAccount* pa = accountIds_.account(account);
        if (pa) jid = pa->jid().bare();
    }
    return jid;
}

QString PluginManager::getId(int account) const {
    QString id = "-1";
    if (accountIds_.isValidRange(account)) {
        id.clear();
        OkAccount* pa = accountIds_.account(account);
        if (pa) id = pa->id();
    }
    qDebug() << "account" << account << "=>id" << id;
    return id;
}

QString PluginManager::getName(int account) const {
    QString nm;
    OkAccount* pa = accountIds_.account(account);
    if (pa) nm = pa->name();
    return nm;
}

QString PluginManager::getPgpKey(int account) const {
    QString keyId;
    //  OkAccount *pa = accountIds_.account(account);
    //  if (pa)
    //    keyId = pa->pgpKeyId();
    return keyId;
}

QMap<QString, QString> PluginManager::getKnownPgpKeys(int account) const {
    QMap<QString, QString> out;
    //  OkAccount *pa = accountIds_.account(account);
    //  if (pa) {
    //    UserAccount acc = pa->userAccount();
    //    for (const auto &item : qAsConst(acc.pgpKnownKeys)) {
    //      out[item.key()] = item.data();
    //    }
    //  }
    return out;
}

int PluginManager::findOnlineAccountForContact(const QString& jid) const {
    //  Jid j(jid);
    //  for (int acc_id = 0; accountIds_.isValidRange(acc_id); ++acc_id) {
    //    OkAccount *pa = accountIds_.account(acc_id);
    //    if (pa && pa->isAvailable() && pa->findContact(j))
    //      return acc_id;
    //  }
    return -1;
}

bool PluginManager::setActivity(int account, const QString& jid, QDomElement xml) {
    //  OkAccount *pa = accountIds_.account(account);
    //  if (!pa)
    //    return false;
    //
    //  XMPP::Jid userJid(jid);
    //  UserListItem *item = pa->userList()->find(userJid);
    //
    //  if (!item)
    //    return false;
    //
    //  Activity act = Activity();
    //  if (!xml.isNull())
    //    act = Activity(xml);
    //  item->setActivity(act);
    return true;
}

bool PluginManager::setMood(int account, const QString& jid, QDomElement xml) {
    //  OkAccount *pa = accountIds_.account(account);
    //  if (!pa)
    //    return false;
    //
    //  XMPP::Jid userJid(jid);
    //  UserListItem *item = pa->userList()->find(userJid);
    //
    //  if (!item)
    //    return false;
    //
    //  Mood mood = Mood();
    //  if (!xml.isNull())
    //    mood = Mood(xml);
    //  item->setMood(mood);
    return true;
}

bool PluginManager::setTune(int account, const QString& jid, const QString& tune) {
    //  OkAccount *pa = accountIds_.account(account);
    //  if (!pa)
    //    return false;
    //
    //  XMPP::Jid userJid(jid);
    //  UserListItem *item = pa->userList()->find(userJid);
    //
    //  if (!item)
    //    return false;
    //
    //  item->setTune(tune);
    return true;
}

void PluginManager::initPopup(const QString& text, const QString& title, const QString& icon,
                              int type) {
    //  const PsiIcon *ico = IconsetFactory::iconPtr(icon);
    //  psi_->popupManager()->doPopup(nullptr, Jid(), ico, title, QPixmap(),
    //  nullptr,
    //                                text, true, PopupManager::PopupType(type));
}

void PluginManager::initPopupForJid(int account, const QString& jid, const QString& text,
                                    const QString& title, const QString& icon, int type) {
    //  XMPP::Jid j(jid);
    //  const PsiIcon *ico = IconsetFactory::iconPtr(icon);
    //  OkAccount *pa = accountIds_.account(account);
    //  if (pa) {
    //    UserListItem *i = pa->findFirstRelevant(j);
    //    PsiIcon *statusIco = PsiIconset::instance()->statusPtr(i);
    //    const QPixmap pix = pa->avatarFactory()->getAvatar(j);
    //    psi_->popupManager()->doPopup(pa, j, ico, title, pix, statusIco, text,
    //    true,
    //                                  PopupManager::PopupType(type));
    //    return;
    //  }
    //  psi_->popupManager()->doPopup(nullptr, Jid(), ico, title, QPixmap(),
    //  nullptr,
    //                                text, true, PopupManager::PopupType(type));
}

int PluginManager::registerOption(const QString& name, int initValue, const QString& path) {
    //  return psi_->popupManager()->registerOption(name, initValue, path);
    return 0;
}

void PluginManager::unregisterOption(const QString& name) {
    // psi_->popupManager()->unregisterOption(name);
}

int PluginManager::popupDuration(const QString& name) const {
    return 0;
    // return psi_->popupManager()->value(name);
}

void PluginManager::setPopupDuration(const QString& name, int value) {
    //    psi_->popupManager()->setValue(name, value);
}

void PluginManager::addAccountMenu(QMenu* menu, OkAccount* account) {
    int i = accountIds_.id(account);
    for (PluginHost* host : qAsConst(pluginsByPriority_)) {
        host->addAccountMenu(menu, i);
    }
}

void PluginManager::addContactMenu(QMenu* menu, OkAccount* account, QString jid) {
    int i = accountIds_.id(account);
    for (PluginHost* host : qAsConst(pluginsByPriority_)) {
        host->addContactMenu(menu, i, jid);
    }
}

void PluginManager::setupChatTab(QWidget* tab, OkAccount* account, const QString& contact) {
    int i = accountIds_.id(account);
    for (PluginHost* host : qAsConst(pluginsByPriority_)) {
        host->setupChatTab(tab, i, contact);
    }
}

void PluginManager::setupGCTab(QWidget* tab, OkAccount* account, const QString& contact) {
    int i = accountIds_.id(account);
    for (PluginHost* host : qAsConst(pluginsByPriority_)) {
        host->setupGCTab(tab, i, contact);
    }
}

bool PluginManager::appendingChatMessage(OkAccount* account, const QString& contact, QString& body,
                                         QDomElement& html, bool local) {
    bool handled = false;
    for (PluginHost* host : qAsConst(pluginsByPriority_)) {
        if (host->appendingChatMessage(accountIds_.id(account), contact, body, html, local)) {
            handled = true;
            break;
        }
    }
    return handled;
}

bool PluginManager::hasInfoProvider(const QString& plugin) const {
    if (hosts_.contains(plugin)) return hosts_[plugin]->hasInfoProvider();

    return false;
}

QString PluginManager::pluginInfo(const QString& plugin) const {
    auto it = hosts_.find(plugin);
    if (it == hosts_.end()) return QString();
    QString info = it.value()->pluginInfo();
    if (info.isEmpty()) info = it.value()->description();
    return info;
}

QIcon PluginManager::icon(const QString& plugin) const {
    QIcon icon;
    if (hosts_.contains(plugin)) {
        icon = hosts_[plugin]->icon();
    }

    return icon;
}

QStringList PluginManager::pluginFeatures() const {
    QStringList features;
    for (PluginHost* host : pluginByFile_) {
        features << host->pluginFeatures();
    }
    return features;
}

/**
 * Tells the plugin manager about add the client OkAccount
 */
int PluginManager::addAccount(OkAccount* account, ::lib::messenger::Messenger* messenger) {
    if (QThread::currentThread() != m_thread.get()) {
        int ret;
        QMetaObject::invokeMethod(this, "addAccount", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(int, ret), Q_ARG(OkAccount*, account),
                                  Q_ARG(::lib::messenger::Messenger*, messenger));
        return ret;
    }

    if (clients_.isEmpty()) {
        clients_.append(messenger);
        const int id = accountIds_.appendAccount(account);
        if (!streamWatcher) {
            streamWatcher = new StreamWatcher(messenger, this, id);
            //  this StreamWatcher instance isn't stored anywhere
            //  and probably leaks (if go(true) isn't called somewhere else)
            connect(account, SIGNAL(accountDestroyed()), this, SLOT(accountDestroyed()));
        }
        return id;
    }
    return 0;
}

/**
 * Performs basic validity checking on a stanza
 * TODO : populate verifyStanza method and use it
 */
bool PluginManager::verifyStanza(const QString& stanza) {
    Q_UNUSED(stanza);
    return true;
}

void PluginManager::applyOptions(const QString& plugin) {
    auto host = hosts_.value(plugin);
    if (host) {
        host->applyOptions();
    }
}

void PluginManager::restoreOptions(const QString& plugin) {
    auto host = hosts_.value(plugin);
    if (host) {
        host->restoreOptions();
    }
}

void PluginManager::addToolBarButton(QObject* parent, QWidget* toolbar, OkAccount* account,
                                     const QString& contact, const QString& plugin) {
    const int acc_id = accountIds_.id(account);
    for (PluginHost* host : qAsConst(pluginsByPriority_)) {
        if ((plugin.isEmpty() || (host->shortName() == plugin)) && host->isEnabled()) {
            host->addToolBarButton(parent, toolbar, acc_id, contact);
        }
    }
}

bool PluginManager::hasToolBarButton(const QString& plugin) const {
    auto host = hosts_.value(plugin);
    if (host && host->isEnabled()) return host->hasToolBarButton();
    return false;
}

void PluginManager::addGCToolBarButton(QObject* parent, QWidget* toolbar, OkAccount* account,
                                       const QString& contact, const QString& plugin) {
    const int acc_id = accountIds_.id(account);
    for (PluginHost* host : qAsConst(pluginsByPriority_)) {
        if ((plugin.isEmpty() || (host->shortName() == plugin)) && host->isEnabled()) {
            host->addGCToolBarButton(parent, toolbar, acc_id, contact);
        }
    }
}

bool PluginManager::hasGCToolBarButton(const QString& plugin) const {
    auto host = hosts_.value(plugin);
    if (host && host->isEnabled()) return host->hasGCToolBarButton();
    return false;
}

void PluginManager::setStatus(int account, const QString& status, const QString& statusMessage) {
    //    OkAccount *acc = accountIds_.account(account);
    //    if (acc) {
    //        XMPP::Status s(status, statusMessage);
    //        acc->setStatus(s, false, true);
    //    }
}

void PluginManager::setPgpKey(int account, const QString& keyId) {
    //    OkAccount *pa = accountIds_.account(account);
    //    if (pa) {
    //        UserAccount acc  = pa->userAccount();
    //        acc.pgpSecretKey = keyId;
    //        pa->setUserAccount(acc);
    //    }
}

void PluginManager::removeKnownPgpKey(int account, const QString& jid) {
    //  OkAccount *pa = accountIds_.account(account);
    //  if (pa) {
    //    pa->removeKnownPgpKey(jid);
    //  }
}

void PluginManager::setClientVersionInfo(int account, const QVariantMap& info) {
    //    OkAccount *pa = accountIds_.account(account);
    //    if (pa) {
    //        pa->setClientVersionInfoMap(info);
    //    }
}

bool PluginManager::appendSysMsg(int account, const QString& jid, const QString& message) {
    //    return appendMsgView(account, jid, MessageView::fromPlainText(message,
    //    MessageView::System));
    return false;
}

bool PluginManager::appendSysHtmlMsg(int account, const QString& jid, const QString& message) {
    //    return appendMsgView(account, jid, MessageView::fromHtml(message,
    //    MessageView::System));
    return false;
}

bool PluginManager::appendMsgView(int account, const QString& jid, const MessageView& message) {
    //    OkAccount *acc = accountIds_.account(account);
    //    if (acc) {
    //        XMPP::Jid j(jid);
    //        ChatDlg * chatDlg = acc->findChatDialogEx(j);
    //        if (!chatDlg) {
    //            chatDlg = acc->findChatDialog(j, false);
    //        }
    //        if (chatDlg) {
    //            chatDlg->dispatchMessage(message);
    //            return true;
    //        }
    //        auto gcDlg = acc->findDialog<GCMainDlg *>(jid);
    //        if (gcDlg) {
    //            gcDlg->dispatchMessage(message);
    //            return true;
    //        }
    //    }
    return false;
}

void PluginManager::createNewEvent(int account, const QString& jid, const QString& descr,
                                   QObject* receiver, const char* slot) {
    //    OkAccount *acc = accountIds_.account(account);
    //    if (acc) {
    //        acc->createNewPluginEvent(account, jid, descr, receiver, slot);
    //    }
}

void PluginManager::createNewMessageEvent(int account, QDomElement const& element) {
    //    OkAccount *acc = accountIds_.account(account);
    //    if (acc) {
    //        acc->createNewMessageEvent(element);
    //    }
}

QList<QAction*> PluginManager::globalAboutMenuActions() const { return QList<QAction*>(); }

bool PluginManager::isSelf(int account, const QString& jid) const {
    //    OkAccount *acc = accountIds_.account(account);
    //    if (acc) {
    //        PsiContact *pc = acc->findContact(XMPP::Jid(jid));
    //        if (pc)
    //            return pc->isSelf();
    //    }
    return false;
}

bool PluginManager::isAgent(int account, const QString& jid) const {
    //    OkAccount *acc = accountIds_.account(account);
    //    if (acc) {
    //        PsiContact *pc = acc->findContact(XMPP::Jid(jid));
    //        if (pc)
    //            return pc->isAgent();
    //    }
    return false;
}

bool PluginManager::inList(int account, const QString& jid) const {
    //    OkAccount *acc = accountIds_.account(account);
    //    if (acc) {
    //        PsiContact *pc = acc->findContact(XMPP::Jid(jid));
    //        if (pc)
    //            return pc->inList();
    //    }
    return false;
}

bool PluginManager::isPrivate(int account, const QString& jid) const {
    //    OkAccount *acc = accountIds_.account(account);
    //    if (acc) {
    //        PsiContact *pc = acc->findContact(XMPP::Jid(jid));
    //        if (pc)
    //            return pc->isPrivate();
    //    }
    return false;
}

bool PluginManager::isConference(int account, const QString& jid) const {
    //    OkAccount *acc = accountIds_.account(account);
    //    if (acc) {
    //        PsiContact *pc = acc->findContact(XMPP::Jid(jid));
    //        if (pc)
    //            return pc->isConference();
    //    }
    return false;
}

QString PluginManager::name(int account, const QString& jid) const {
    //    OkAccount *acc = accountIds_.account(account);
    //    if (acc) {
    //        PsiContact *pc = acc->findContact(XMPP::Jid(jid));
    //        if (pc)
    //            return pc->name();
    //    }
    return QString();
}

QString PluginManager::status(int account, const QString& jid) const {
    //    OkAccount *acc = accountIds_.account(account);
    //    if (acc) {
    //        PsiContact *pc = acc->findContact(XMPP::Jid(jid));
    //        if (pc)
    //            return pc->status().typeString();
    //    }
    return QString();
}

QString PluginManager::statusMessage(int account, const QString& jid) const {
    //    OkAccount *acc = accountIds_.account(account);
    //    if (acc) {
    //        PsiContact *pc = acc->findContact(XMPP::Jid(jid));
    //        if (pc)
    //            return pc->status().status();
    //    }
    return QString();
}

QStringList PluginManager::resources(int account, const QString& jid) const {
    QStringList l;
    //    OkAccount *pa = accountIds_.account(account);
    //    if (pa) {
    //        UserListItem *u = pa->findFirstRelevant(XMPP::Jid(jid));
    //        if (u) {
    //            QMutableListIterator<UserResource> i(u->userResourceList());
    //            while (i.hasNext()) {
    //                l.push_back(i.next().name());
    //            }
    //        }
    //    }
    return l;
}

QString PluginManager::realJid(int account, const QString& jid) const {
    OkAccount* acc = accountIds_.account(account);
    if (acc) {
        ok::base::Jid realJid = acc->realJid(ok::base::Jid(jid));
        return realJid.isNull() ? jid : realJid.full();
    }
    return jid;
}

QString PluginManager::mucNick(int account, const QString& mucJid) const {
    //    OkAccount *acc = accountIds_.account(account);
    //    if (acc) {
    //        auto gcDlg = acc->findDialog<GCMainDlg *>(mucJid);
    //        if (gcDlg) {
    //            return gcDlg->nick();
    //        }
    //    }
    return "";
}

QStringList PluginManager::mucNicks(int account, const QString& mucJid) const {
    //    OkAccount *acc = accountIds_.account(account);
    //    if (acc) {
    //        auto gcDlg = acc->findDialog<GCMainDlg *>(mucJid);
    //        if (gcDlg) {
    //            return gcDlg->mucRosterContent();
    //        }
    //    }
    return {};
}

bool PluginManager::hasCaps(int account, const QString& jid, const QStringList& caps) {
    //        QStringList l;
    //        OkAccount *pa = accountIds_.account(account);
    //        if (pa) {
    //            return pa->client()->capsManager()->features(jid).test(caps);
    //        }
    return false;
}

bool PluginManager::decryptMessageElement(OkAccount* account, QDomElement& message) {
    if (QThread::currentThread() != m_thread.get()) {
        bool ret;
        QMetaObject::invokeMethod(this,
                                  "decryptMessageElement",
                                  Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(bool, ret),
                                  Q_ARG(OkAccount*, account),
                                  Q_ARG(QDomElement&, message));
        return ret;
    }

    for (auto const host : pluginByFile_) {
        if (host->decryptMessageElement(accountIds_.id(account), message)) {
            return true;
        }
    }
    return false;
}

bool PluginManager::encryptMessageElement(OkAccount* account, QDomElement& message) {
    if (QThread::currentThread() != m_thread.get()) {
        bool ret;
        QMetaObject::invokeMethod(this,
                                  "encryptMessageElement",
                                  Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(bool, ret),
                                  Q_ARG(OkAccount*, account),
                                  Q_ARG(QDomElement&, message));
        return ret;
    }

    qDebug() << "encryptMessageElement:" << account->getUsername()
             << message.ownerDocument().toString();
    for (auto const host : pluginByFile_) {
        if (host->encryptMessageElement(accountIds_.id(account), message)) {
            qDebug() << "Using plugin:" << host->shortName() << "encryptMessageElement completed.";
            return true;
        }
    }
    qDebug() << "Would not find suitable plugin!";
    return false;
}

int AccountIds::appendAccount(OkAccount* acc) {
    int id = -1;
    if (acc) {
        id = id_keys.size();
        id_keys[id] = acc;
        acc_keys[acc] = id;
    }
    return id;
}

void AccountIds::removeAccount(ok::base::OkAccount* acc) {
    int id = acc_keys.value(acc, -1);
    if (id != -1) {
        acc_keys.remove(acc);
        id_keys[id] = nullptr;
    }
}

void AccountIds::clear() {
    acc_keys.clear();
    id_keys.clear();
}

OkAccount* AccountIds::account(int id) const { return id_keys.value(id, nullptr); }

int AccountIds::id(OkAccount* acc) const { return acc_keys.value(acc, -1); }

PluginManager* PluginManager::instance_ = nullptr;
const QString PluginManager::loadOptionPrefix = "plugins.auto-load";
const QString PluginManager::pluginOptionPrefix = "plugins.options";

PluginHost* PluginManager::plugin(const QString& name) const { return hosts_.value(name); }

}  // namespace plugin
}  // namespace ok
