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

#ifndef SETTINGS_HPP
#define SETTINGS_HPP


#include "lib/audio/iaudiosettings.h"
#include "src/core/icoresettings.h"
#include "src/core/toxfile.h"
#include "src/persistence/ifriendsettings.h"
#include "src/persistence/igroupsettings.h"

#include "base/compatiblerecursivemutex.h"

#include <QDateTime>
#include <QFlags>
#include <QFont>
#include <QHash>
#include <QMutex>
#include <QNetworkProxy>
#include <QObject>
#include <QPixmap>
#include <QSettings>

#include "src/model/contactid.h"

class QCommandLineParser;

namespace Db {
enum class syncType;
}

namespace module::im {

class Profile;

class Settings : public QObject,
                 public ICoreSettings,
                 public IFriendSettings,
                 public IGroupSettings {
    Q_OBJECT

    Q_ENUMS(StyleType)

    // GUI
    Q_PROPERTY(bool separateWindow READ getSeparateWindow WRITE setSeparateWindow NOTIFY
                       separateWindowChanged FINAL)
    Q_PROPERTY(QString smileyPack READ getSmileyPack WRITE setSmileyPack NOTIFY smileyPackChanged
                       FINAL)
    Q_PROPERTY(int emojiFontPointSize READ getEmojiFontPointSize WRITE setEmojiFontPointSize NOTIFY
                       emojiFontPointSizeChanged FINAL)

    Q_PROPERTY(QByteArray splitterState READ getSplitterState WRITE setSplitterState NOTIFY
                       splitterStateChanged FINAL)
    Q_PROPERTY(QByteArray dialogGeometry READ getDialogGeometry WRITE setDialogGeometry NOTIFY
                       dialogGeometryChanged FINAL)
    Q_PROPERTY(QByteArray dialogSplitterState READ getDialogSplitterState WRITE
                       setDialogSplitterState NOTIFY dialogSplitterStateChanged FINAL)
    Q_PROPERTY(QByteArray dialogSettingsGeometry READ getDialogSettingsGeometry WRITE
                       setDialogSettingsGeometry NOTIFY dialogSettingsGeometryChanged FINAL)
    Q_PROPERTY(QString style READ getStyle WRITE setStyle NOTIFY styleChanged FINAL)

    Q_PROPERTY(bool showIdenticons READ getShowIdenticons WRITE setShowIdenticons NOTIFY
                       showIdenticonsChanged FINAL)

    // ChatView
    Q_PROPERTY(bool groupchatPosition READ getGroupchatPosition WRITE setGroupchatPosition NOTIFY
                       groupchatPositionChanged FINAL)
    Q_PROPERTY(QFont chatMessageFont READ getChatMessageFont WRITE setChatMessageFont NOTIFY
                       chatMessageFontChanged FINAL)
    Q_PROPERTY(StyleType stylePreference READ getStylePreference WRITE setStylePreference NOTIFY
                       stylePreferenceChanged FINAL)

    Q_PROPERTY(bool statusChangeNotificationEnabled READ getStatusChangeNotificationEnabled WRITE
                       setStatusChangeNotificationEnabled NOTIFY
                               statusChangeNotificationEnabledChanged FINAL)
    Q_PROPERTY(bool spellCheckingEnabled READ getSpellCheckingEnabled WRITE setSpellCheckingEnabled
                       NOTIFY spellCheckingEnabledChanged FINAL)

    // Privacy
    Q_PROPERTY(bool typingNotification READ getTypingNotification WRITE setTypingNotification NOTIFY
                       typingNotificationChanged FINAL)

public:
    enum class StyleType { NONE = 0, WITH_CHARS = 1, WITHOUT_CHARS = 2 };

    enum class FriendListSortingMode {
        Name,
        Activity,
    };

public:
    explicit Settings(QSettings* s);
    ~Settings();

    static void destroyInstance();
    QString getSettingsDirPath() const;
    QString getAppDataDirPath() const;

    void createSettingsDir();

    void loadGlobal();

    void resetToDefault();

    //    QStringList getLocales();

    struct Request {
        QString address;
        QString message;
        bool read;
    };

public slots:
    void saveGlobal();
    void sync();
    void setAutoLogin(bool state);
    //    void updateProfileData(Profile* profile, const QCommandLineParser* parser);

signals:
    // General
    void lightTrayIconChanged(bool enabled);
    void notifyChanged(bool enabled);
    void desktopNotifyChanged(bool enabled);
    void showWindowChanged(bool enabled);
    void makeToxPortableChanged(bool enabled);
    void busySoundChanged(bool enabled);
    void notifySoundChanged(bool enabled);
    void notifyHideChanged(bool enabled);
    void groupAlwaysNotifyChanged(bool enabled);

    void autoAwayTimeChanged(int minutes);
    void globalAutoAcceptDirChanged(const QString& path);
    void autoAcceptMaxSizeChanged(size_t size);
    void checkUpdatesChanged(bool enabled);
    void widgetDataChanged(const QString& key);

    // GUI
    void autoLoginChanged(bool enabled);
    void nameColorsChanged(bool enabled);
    void separateWindowChanged(bool enabled);
    void windowGeometryChanged(const QByteArray& rect);
    void windowStateChanged(const QByteArray& state);
    void splitterStateChanged(const QByteArray& state);
    void dialogGeometryChanged(const QByteArray& rect);
    void dialogSplitterStateChanged(const QByteArray& state);
    void dialogSettingsGeometryChanged(const QByteArray& rect);
    void styleChanged(const QString& style);
    void themeColorChanged(int color);

    void sortingModeChanged(FriendListSortingMode mode);
    void showIdenticonsChanged(bool enabled);

    // ChatView
    void useEmoticonsChanged(bool enabled);
    void smileyPackChanged(const QString& name);
    void emojiFontPointSizeChanged(int size);
    void dontGroupWindowsChanged(bool enabled);
    void groupchatPositionChanged(bool enabled);
    void chatMessageFontChanged(const QFont& font);
    void stylePreferenceChanged(StyleType type);

    void statusChangeNotificationEnabledChanged(bool enabled);
    void spellCheckingEnabledChanged(bool enabled);

    // Privacy
    void typingNotificationChanged(bool enabled);
    void dbSyncTypeChanged(Db::syncType type);

public:
    bool applyCommandLineOptions(const QCommandLineParser& parser);
    static bool verifyProxySettings(const QCommandLineParser& parser);

    bool getLightTrayIcon() const;
    void setLightTrayIcon(bool newValue);

    QString getStyle() const;
    void setStyle(const QString& newValue);

    bool getUseEmoticons() const;
    void setUseEmoticons(bool newValue);

    QString getTranslation() const;
    void setTranslation(const QString& newValue);

    // ICoreSettings
    bool getEnableIPv6() const override;
    void setEnableIPv6(bool enabled) override;

    bool getForceTCP() const override;
    void setForceTCP(bool enabled) override;

    bool getEnableLanDiscovery() const override;
    void setEnableLanDiscovery(bool enabled) override;

    QString getProxyAddr() const override;
    void setProxyAddr(const QString& address) override;

    ICoreSettings::ProxyType getProxyType() const override;
    void setProxyType(ICoreSettings::ProxyType type) override;

    quint16 getProxyPort() const override;
    void setProxyPort(quint16 port) override;

    QNetworkProxy getProxy() const override;

    SIGNAL_IMPL(Settings, enableIPv6Changed, bool enabled)
    SIGNAL_IMPL(Settings, forceTCPChanged, bool enabled)
    SIGNAL_IMPL(Settings, enableLanDiscoveryChanged, bool enabled)
    SIGNAL_IMPL(Settings, proxyTypeChanged, ICoreSettings::ProxyType type)
    SIGNAL_IMPL(Settings, proxyAddressChanged, const QString& address)
    SIGNAL_IMPL(Settings, proxyPortChanged, quint16 port)

    Db::syncType getDbSyncType() const;
    void setDbSyncType(Db::syncType newValue);

    int getAutoAwayTime() const;
    void setAutoAwayTime(int newValue);

    bool getCheckUpdates() const;
    void setCheckUpdates(bool newValue);

    bool getNotify() const;
    void setNotify(bool newValue);

    bool getShowWindow() const;
    void setShowWindow(bool newValue);

    bool getDesktopNotify() const;
    void setDesktopNotify(bool enabled);

    bool getNotifySound() const;
    void setNotifySound(bool newValue);

    bool getNotifyHide() const;
    void setNotifyHide(bool newValue);

    bool getBusySound() const;
    void setBusySound(bool newValue);

    bool getGroupAlwaysNotify() const override;
    void setGroupAlwaysNotify(bool newValue) override;

    bool isAnimationEnabled() const;
    void setAnimationEnabled(bool newValue);

    QString getSmileyPack() const;
    void setSmileyPack(const QString& value);

    int getThemeColor() const;
    void setThemeColor(int value);

    StyleType getStylePreference() const;
    void setStylePreference(StyleType newValue);

    bool isCurstomEmojiFont() const;
    void setCurstomEmojiFont(bool value);

    int getEmojiFontPointSize() const;
    void setEmojiFontPointSize(int value);

    QString getContactNote(const FriendId& id) const override;
    void setContactNote(const FriendId& id, const QString& note) override;

    QString getAutoAcceptDir(const FriendId& id) const override;
    void setAutoAcceptDir(const FriendId& id, const QString& dir) override;

    AutoAcceptCallFlags getAutoAcceptCall(const FriendId& id) const override;
    void setAutoAcceptCall(const FriendId& id, AutoAcceptCallFlags accept) override;

    QString getGlobalAutoAcceptDir() const;
    void setGlobalAutoAcceptDir(const QString& dir);

    size_t getMaxAutoAcceptSize() const;
    void setMaxAutoAcceptSize(size_t size);

    bool getAutoGroupInvite(const FriendId& id) const override;
    void setAutoGroupInvite(const FriendId& id, bool accept) override;

    // ChatView
    const QFont& getChatMessageFont() const;
    void setChatMessageFont(const QFont& font);

    bool getStatusChangeNotificationEnabled() const;
    void setStatusChangeNotificationEnabled(bool newValue);

    bool getSpellCheckingEnabled() const;
    void setSpellCheckingEnabled(bool newValue);

    // Privacy
    bool getTypingNotification() const;
    void setTypingNotification(bool enabled);

    bool getShowGroupJoinLeaveMessages() const override;
    void setShowGroupJoinLeaveMessages(bool newValue) override;
    SIGNAL_IMPL(Settings, showGroupJoinLeaveMessagesChanged, bool show)

    // State
    QByteArray getWindowGeometry() const;
    void setWindowGeometry(const QByteArray& value);

    QByteArray getWindowState() const;
    void setWindowState(const QByteArray& value);

    QByteArray getSplitterState() const;
    void setSplitterState(const QByteArray& value);

    QByteArray getDialogGeometry() const;
    void setDialogGeometry(const QByteArray& value);

    QByteArray getDialogSplitterState() const;
    void setDialogSplitterState(const QByteArray& value);

    QByteArray getDialogSettingsGeometry() const;
    void setDialogSettingsGeometry(const QByteArray& value);

    QString getFriendAddress(const QString& publicKey) const;
    void updateFriendAddress(const QString& newAddr);

    QString getFriendAlias(const ContactId& id) const override;
    void setFriendAlias(const FriendId& id, const QString& alias) override;

    int getFriendCircleID(const FriendId& id) const override;
    void setFriendCircleID(const FriendId& id, int circleID) override;

    QDateTime getFriendActivity(const FriendId& id) const override;
    void setFriendActivity(const FriendId& id, const QDateTime& date) override;

    void saveFriendSettings(const FriendId& id) override;
    void removeFriendSettings(const FriendId& id) override;

    SIGNAL_IMPL(Settings, autoAcceptCallChanged, const FriendId& id,
                IFriendSettings::AutoAcceptCallFlags accept)
    SIGNAL_IMPL(Settings, autoGroupInviteChanged, const FriendId& id, bool accept)
    SIGNAL_IMPL(Settings, autoAcceptDirChanged, const FriendId& id, const QString& dir)
    SIGNAL_IMPL(Settings, contactNoteChanged, const FriendId& id, const QString& note)

    FriendListSortingMode getFriendSortingMode() const;
    void setFriendSortingMode(FriendListSortingMode mode);

    bool getSeparateWindow() const;
    void setSeparateWindow(bool value);

    bool getDontGroupWindows() const;
    void setDontGroupWindows(bool value);

    bool getGroupchatPosition() const;
    void setGroupchatPosition(bool value);

    bool getShowIdenticons() const;
    void setShowIdenticons(bool value);

    bool getAutoLogin() const;
    void setEnableGroupChatsColor(bool state);
    bool getEnableGroupChatsColor() const;

    int getCircleCount() const;
    int addCircle(const QString& name = QString());
    int removeCircle(int id);
    QString getCircleName(int id) const;
    void setCircleName(int id, const QString& name);
    bool getCircleExpanded(int id) const;
    void setCircleExpanded(int id, bool expanded);

    bool addFriendRequest(const QString& friendAddress, const QString& message);
    unsigned int getUnreadFriendRequests() const;
    Request getFriendRequest(int index) const;
    int getFriendRequestSize() const;
    void clearUnreadFriendRequests();
    void removeFriendRequest(int index);
    void readFriendRequest(int index);

    QByteArray getWidgetData(const QString& uniqueName) const;
    void setWidgetData(const QString& uniqueName, const QByteArray& data);

    // Wrappers around getWidgetData() and setWidgetData()
    // Assume widget has a unique objectName set
    template <class T> void restoreGeometryState(T* widget) const {
        widget->restoreGeometry(getWidgetData(widget->objectName() + "Geometry"));
        widget->restoreState(getWidgetData(widget->objectName() + "State"));
    }
    template <class T> void saveGeometryState(const T* widget) {
        setWidgetData(widget->objectName() + "Geometry", widget->saveGeometry());
        setWidgetData(widget->objectName() + "State", widget->saveState());
    }

private:
    struct friendProp;

    Settings(Settings& settings) = delete;
    Settings& operator=(const Settings&) = delete;
    friendProp& getOrInsertFriendPropRef(const FriendId& id);
    ICoreSettings::ProxyType fixInvalidProxyType(ICoreSettings::ProxyType proxyType);

    QSettings* s;
    bool loaded;

    bool autoLogin;
    bool compactLayout;
    FriendListSortingMode sortingMode;
    bool groupchatPosition;
    bool separateWindow;
    bool dontGroupWindows;
    bool showIdenticons;
    bool enableIPv6;
    //    QString translation;
    bool makeToxPortable;

    bool lightTrayIcon;
    bool useEmoticons;
    bool checkUpdates;
    bool notify;
    bool desktopNotify;
    bool showWindow;
    bool notifySound;
    bool notifyHide;
    bool busySound;
    bool groupAlwaysNotify;
    bool nameColors;

    bool forceTCP;
    bool enableLanDiscovery;

    ICoreSettings::ProxyType proxyType;
    QString proxyAddr;
    quint16 proxyPort;

    int autoAwayTime;

    QHash<QString, QByteArray> widgetSettings;
    QHash<QString, QString> autoAccept;
    QString globalAutoAcceptDir;
    size_t autoAcceptMaxSize;

    QList<Request> friendRequests;

    // GUI 通用
    QString smileyPack;
    int emojiFontPointSize;
    QByteArray windowGeometry;
    QByteArray windowState;
    QByteArray splitterState;
    QByteArray dialogGeometry;
    QByteArray dialogSplitterState;
    QByteArray dialogSettingsGeometry;
    QString style;

    // ChatView 用户界面
    QFont chatMessageFont;
    StyleType stylePreference;
    int firstColumnHandlePos;
    int secondColumnHandlePosFromRight;
    QString timestampFormat;
    QString dateFormat;
    bool statusChangeNotificationEnabled;
    bool showGroupJoinLeaveMessages;
    bool spellCheckingEnabled;

    // Privacy 隐私
    bool typingNotification;
    Db::syncType dbSyncType;

    struct friendProp {
        friendProp() = delete;
        friendProp(QString addr) : addr(addr) {}
        QString alias = "";
        QString addr = "";
        QString autoAcceptDir = "";
        QString note = "";
        int circleID = -1;
        QDateTime activity = QDateTime();
        AutoAcceptCallFlags autoAcceptCall;
        bool autoGroupInvite = false;
    };

    struct circleProp {
        QString name;
        bool expanded;
    };

    QHash<QByteArray, friendProp> friendLst;

    QVector<circleProp> circleLst;

    int themeColor;

    static CompatibleRecursiveMutex bigLock;
    static QThread* settingsThread;
};
}  // namespace module::im
#endif  // SETTINGS_HPP
