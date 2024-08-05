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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include <QDomElement>
#include <QHash>
#include <QList>
#include <QMap>
#include <QMenu>
#include <QtCore>

class QPluginLoader;
class Profile;

namespace lib {
namespace messenger {
class Messenger;
}
}  // namespace lib

namespace ok {

namespace base {
class OkAccount;
}

namespace plugin {

class MessageView;
class PluginHost;

class PsiCon;
class OkPlugin;
class OkOptions;
class OAH_PluginOptionsTab;

namespace QCA {
class DirWatch;
}

namespace XMPP {
class Client;
}

namespace PsiMedia {
class Provider;
}

using namespace ok::base;

class AccountIds {
public:
    int appendAccount(OkAccount* acc);
    void removeAccount(OkAccount* acc);
    void clear();
    bool isValidRange(int id) const { return id_keys.contains(id); }
    OkAccount* account(int id) const;
    int id(OkAccount* acc) const;

private:
    QHash<OkAccount*, int> acc_keys;
    QHash<int, OkAccount*> id_keys;
};

class PluginManager : public QObject {
    Q_OBJECT
public:
    static PluginManager* instance();
    void initNewSession(PsiCon* psi);

    QStringList pluginDirs();
    QStringList availablePlugins() const;
    PluginHost* plugin(const QString& name) const;

    /**
     * install & uninstall plugins
     * @param file
     * @return
     */
    bool installPlugin(const QString& filePath, const QString& fileName);

    bool uninstallPlugin(const QString& shortName);

    void loadEnabledPlugins();
    bool unloadAllPlugins();

    bool isAvailable(const QString& plugin) const;
    bool isEnabled(const QString& plugin) const;
    QString pathToPlugin(const QString& plugin) const;
    QString shortName(const QString& plugin) const;
    QString pluginName(const QString& shortName) const;
    QString version(const QString& plugin) const;
    QString vendor(const QString& plugin) const;
    QString description(const QString& plugin) const;
    QWidget* optionsWidget(const QString& plugin);

    void setShortcuts();

    bool processEvent(OkAccount* account, QDomElement& eventXml);
    bool processMessage(OkAccount* account, const QString& jidFrom, const QString& body,
                        const QString& subject);
    bool processOutgoingMessage(OkAccount* account, const QString& jidTo, QString& body,
                                const QString& type, QString& subject);
    void processOutgoingStanza(OkAccount* account, QDomElement& stanza);
    void startLogin(OkAccount* account);
    void logout(OkAccount* account);

    void addSettingPage(OAH_PluginOptionsTab* tab);
    void removeSettingPage(OAH_PluginOptionsTab* tab);
    QList<OAH_PluginOptionsTab*> settingsPages() const;
    void applyOptions(const QString& plugin);
    void restoreOptions(const QString& plugin);
    void addToolBarButton(QObject* parent, QWidget* toolbar, OkAccount* account,
                          const QString& contact, const QString& plugin = "");
    bool hasToolBarButton(const QString& plugin) const;
    void addGCToolBarButton(QObject* parent, QWidget* toolbar, OkAccount* account,
                            const QString& contact, const QString& plugin = "");
    bool hasGCToolBarButton(const QString& plugin) const;
    void addAccountMenu(QMenu* menu, OkAccount* account);
    void addContactMenu(QMenu* menu, OkAccount* account, QString jid);

    void setupChatTab(QWidget* tab, OkAccount* account, const QString& contact);
    void setupGCTab(QWidget* tab, OkAccount* account, const QString& contact);
    bool appendingChatMessage(OkAccount* account, const QString& contact, QString& body,
                              QDomElement& html, bool local);

    QString pluginInfo(const QString& plugin) const;
    bool hasInfoProvider(const QString& plugin) const;
    QIcon icon(const QString& plugin) const;
    QStringList pluginFeatures() const;

    QList<QAction*> globalAboutMenuActions() const;

    static const QString loadOptionPrefix;
    static const QString pluginOptionPrefix;

signals:
    void jsFiltersUpdated();
    void accountBeforeLogin(int account_id);
    void accountLoggedOut(int account_id);
    void pluginEnabled(const QString& shortName);
    void pluginDisabled(const QString& shortName);

private:
    PluginManager();
    PsiCon* psi_;
    void loadAllPlugins();
    bool verifyStanza(const QString& stanza);
    QList<PluginHost*> updatePluginsList();
    void loadPluginIfEnabled(PluginHost* host);
    PluginHost* addHostFile(const QString& file);
    void removeHostFile(PluginHost* host);

    static PluginManager* instance_;

    // plugin manager thread
    std::unique_ptr<QThread> m_thread;

    // account id, client
    QVector<::lib::messenger::Messenger*> clients_;

    // account, account id
    AccountIds accountIds_;

    // shortName, host
    QMap<QString, PluginHost*> hosts_;
    // file, host
    QMap<QString, PluginHost*> pluginByFile_;
    // sorted by priority
    QList<PluginHost*> pluginsByPriority_;

    QList<QCA::DirWatch*> dirWatchers_;

    // Options widget provides by plugin on opt_plugins
    QPointer<QWidget> optionsWidget_;
    QList<OAH_PluginOptionsTab*> settingsTabs_;  // to be inserted into global list

    // QMultiMap<OkPlugin::Priority, std::pair<QString, QString>>
    // _messageViewJSFilters; // priority -> <js, uuid>
    QTimer* _messageViewJSFiltersTimer = nullptr;

    class StreamWatcher;

    StreamWatcher* streamWatcher = nullptr;

    bool incomingXml(int account, const QDomElement& xml);
    void sendXml(int account, const QString& xml);
    QString uniqueId(int account) const;

    QString getStatus(int account) const;
    QString getStatusMessage(int account) const;
    QString proxyHost(int account) const;
    int proxyPort(int account) const;
    QString proxyUser(int account) const;
    QString proxyPassword(int account) const;
    QStringList getRoster(int account) const;
    QString getJid(int account) const;
    QString getId(int account) const;
    QString getName(int account) const;
    int findOnlineAccountForContact(const QString& jid) const;
    QString getPgpKey(int account) const;
    QMap<QString, QString> getKnownPgpKeys(int account) const;

    bool isSelf(int account, const QString& jid) const;
    bool isAgent(int account, const QString& jid) const;
    bool inList(int account, const QString& jid) const;
    bool isPrivate(int account, const QString& jid) const;
    bool isConference(int account, const QString& jid) const;
    QString name(int account, const QString& jid) const;
    QString status(int account, const QString& jid) const;
    QString statusMessage(int account, const QString& jid) const;
    QStringList resources(int account, const QString& jid) const;
    QString realJid(int account, const QString& jid) const;
    QString mucNick(int account, const QString& mucJid) const;
    QStringList mucNicks(int account, const QString& mucJid) const;
    bool hasCaps(int account, const QString& jid, const QStringList& caps);

    bool setActivity(int account, const QString& Jid, QDomElement xml);
    bool setMood(int account, const QString& Jid, QDomElement xml);
    bool setTune(int account, const QString& Jid, const QString& tune);

    void initPopup(const QString& text, const QString& title, const QString& icon, int type);
    void initPopupForJid(int account, const QString& jid, const QString& text, const QString& title,
                         const QString& icon, int tipe);
    int registerOption(const QString& name, int initValue = 5, const QString& path = QString());
    void unregisterOption(const QString& name);
    int popupDuration(const QString& name) const;
    void setPopupDuration(const QString& name, int value);

    void setStatus(int account, const QString& status, const QString& statusMessage);
    void setPgpKey(int account, const QString& keyId);
    void removeKnownPgpKey(int account, const QString& jid);
    void setClientVersionInfo(int account, const QVariantMap& info);

    bool appendSysMsg(int account, const QString& jid, const QString& message);
    bool appendSysHtmlMsg(int account, const QString& jid, const QString& message);

    void createNewEvent(int account, const QString& jid, const QString& descr, QObject* receiver,
                        const char* slot);
    void createNewMessageEvent(int account, QDomElement const& element);

    void updateFeatures();

    // QString installChatLogJSDataFilter(
    //     const QString &js,
    //     OkPlugin::Priority priority = OkPlugin::PriorityNormal);

    void uninstallChatLogJSDataFilter(const QString& id);

    friend class PluginHost;

private:
    bool appendMsgView(int account, const QString& jid, const MessageView& message);
signals:

private slots:
    void dirsChanged();
    void optionChanged(const QString& option);
    void accountDestroyed();

public slots:
    int addAccount(OkAccount* account, ::lib::messenger::Messenger* messenger);
    bool decryptMessageElement(OkAccount* account, QDomElement& message);
    bool encryptMessageElement(OkAccount* account, QDomElement& message);
};
}  // namespace plugin
}  // namespace ok
#endif  // PLUGINMANAGER_H
