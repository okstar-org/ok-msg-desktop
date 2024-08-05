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

#ifndef COMMON_H
#define COMMON_H

#include <QColor>
#include <QGridLayout>
#include <QList>
#include <QMap>
#include <QSize>
#include <QString>
#include <QStringList>
#include <functional>

#if __cplusplus < 201703L
#define PSI_FALLSTHROUGH
#else
#define PSI_FALLSTHROUGH [[fallthrough]]
#endif

class QMenu;
class TabbableWidget;
class QPaintDevice;

// -----------------------------------------------------------------------------
// Options
// -----------------------------------------------------------------------------

enum { dcClose, dcHour, dcDay, dcNever };

enum Qt3Dock {
    Qt3Dock_Unmanaged = 0,
    Qt3Dock_TornOff = 1,
    Qt3Dock_Top = 2,
    Qt3Dock_Bottom = 3,
    Qt3Dock_Right = 4,
    Qt3Dock_Left = 5,
    Qt3Dock_Minimized = 6
};

class ToolbarPrefs {
public:
    ToolbarPrefs();

    QString id;
    QString name;
    QStringList keys;
    Qt3Dock dock;

    // bool dirty;
    bool on;
    bool locked;
    // bool stretchable;

    // int index;
    bool nl;
    // int extraOffset;

    bool operator==(const ToolbarPrefs& other);
};

struct lateMigrationOptions {
    QMap<QString, QString> serviceRosterIconset;
    QMap<QString, QString> customRosterIconset;
    QMap<QString, QList<ToolbarPrefs>> toolbars;
};

// used to be part of the global options struct.
// do not modify or save/load this value! it is calculated at run time!
// FIXME find it a new home!
extern int common_smallFontSize;

// used to be part of the global options struct.
// FIXME find it a new home!
enum { EventPriorityDontCare = -1 };

// coofficients icon-size / font-size signalling where icon needs to be scaled-down
constexpr double HugeIconRosterK = 2.0;
constexpr double HugeIconButtonK = 2.0;
constexpr double HugeIconTextViewK = 1.5;

constexpr double EqTextIconK = 0.93;     // icon size equal to text
constexpr double BiggerTextIconK = 1.5;  // icon visually bigger than text but still looks good

int pointToPixel(qreal points);
int computeScaleFactor(QPaintDevice* pd);

// -----------------------------------------------------------------------------
// Status
// -----------------------------------------------------------------------------

#define STATUS_OFFLINE XMPP::Status::Offline
#define STATUS_ONLINE XMPP::Status::Online
#define STATUS_AWAY XMPP::Status::Away
#define STATUS_XA XMPP::Status::XA
#define STATUS_DND XMPP::Status::DND
#define STATUS_INVISIBLE XMPP::Status::Invisible
#define STATUS_CHAT XMPP::Status::FFC

#define STATUS_ASK 100
#define STATUS_NOAUTH 101
#define STATUS_ERROR 102

QString status2txt(int status);
bool lastPriorityNotEmpty();
QString clipStatus(const QString& str, int width, int height);

// -----------------------------------------------------------------------------
// Widget tools
// -----------------------------------------------------------------------------

bool isKde();
void clearMenu(QMenu* m);  // deletes all items, including submenus, from given QMenu
void bringToFront(QWidget* w, bool grabFocus = true);
void replaceWidget(QWidget*, QWidget*);
void closeDialogs(QWidget*);
TabbableWidget* findActiveTab();
#ifdef HAVE_X11
#include "x11windowsystem.h"
#define X11WM_CLASS(x)                                                                        \
    {                                                                                         \
        if (QX11Info::isPlatformX11()) X11WindowSystem::instance()->x11wmClass(winId(), (x)); \
    };
#else
#define X11WM_CLASS(x) /* dummy */
#endif
void reorderGridLayout(QGridLayout* layout, int maxCols);

// -----------------------------------------------------------------------------
// History utilities
// -----------------------------------------------------------------------------

QString logencode(QString);
QString logdecode(const QString&);

// -----------------------------------------------------------------------------
// Misc.
// -----------------------------------------------------------------------------

QString CAP(const QString& str);

QString encodePassword(const QString&, const QString&);
QString decodePassword(const QString&, const QString&);
#ifdef HAVE_KEYCHAIN
bool isKeychainEnabled();
#endif

bool operator!=(const QMap<QString, QString>&, const QMap<QString, QString>&);

bool fileCopy(const QString& src, const QString& dest);

extern Qt::WindowFlags psi_dialog_flags;

// like QT_VERSION, but runtime
int qVersionInt();

#endif  // COMMON_H
