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

#include "common.h"
#include "lib/settings/applicationinfo.h"

#ifdef Q_OS_MAC
// #include "CocoaUtilities/cocoacommon.h"
#endif
#ifdef HAVE_X11
#include "x11windowsystem.h"
#endif

#include <QApplication>
#include <QBoxLayout>
#include <QDesktopWidget>
#include <QDir>
#include <QFile>
#include <QLibrary>
#include <QMenu>
#include <QMessageBox>
#include <QObject>
#include <QPaintDevice>
#include <QProcess>
#include <QRegExp>
#include <QSound>
#include <QUrl>
#include <QUuid>
#ifdef __GLIBC__
#include <langinfo.h>
#endif
#ifdef HAVE_KEYCHAIN
#include <qt5keychain/keychain.h>
#endif
#include <stdio.h>
#ifdef Q_OS_MAC
#include <Carbon/Carbon.h>  // for HIToolbox/InternetConfig
#include <CoreServices/CoreServices.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif
#ifdef Q_OS_WIN
#include <windows.h>
#endif

Qt::WindowFlags psi_dialog_flags = (Qt::WindowSystemMenuHint | Qt::WindowMinMaxButtonsHint);

// used to be part of the global options struct.
// FIXME find it a new home!
int common_smallFontSize = 0;

QString CAP(const QString& str) { return QString("%1: %2").arg(ApplicationInfo::name(), str); }

// clips plain text
QString clipStatus(const QString& str, int width, int height) {
    QString out = "";
    int at = 0;
    int len = str.length();
    if (len == 0) return out;

    // only take the first "height" lines
    for (int n2 = 0; n2 < height; ++n2) {
        // only take the first "width" chars
        QString line;
        bool hasNewline = false;
        for (int n = 0; at < len; ++n, ++at) {
            if (str.at(at) == '\n') {
                hasNewline = true;
                break;
            }
            line += str.at(at);
        }
        ++at;
        if (int(line.length()) > width) {
            line.truncate(width - 3);
            line += "...";
        }
        out += line;
        if (hasNewline) {
            out += '\n';
        }

        if (at >= len) {
            break;
        }
    }

    return out;
}

QString encodePassword(const QString& pass, const QString& key) {
    QString result;
    int n1, n2;

    if (key.length() == 0) {
        return pass;
    }

    for (n1 = 0, n2 = 0; n1 < pass.length(); ++n1) {
        ushort x = pass.at(n1).unicode() ^ key.at(n2++).unicode();
        QString hex = QString::asprintf("%04x", x);
        result += hex;
        if (n2 >= key.length()) {
            n2 = 0;
        }
    }
    return result;
}

QString decodePassword(const QString& pass, const QString& key) {
    QString result;
    int n1, n2;

    if (key.length() == 0) {
        return pass;
    }

    for (n1 = 0, n2 = 0; n1 < pass.length(); n1 += 4) {
        ushort x = 0;
        if (n1 + 4 > pass.length()) {
            break;
        }
        x += QString(pass.at(n1)).toInt(nullptr, 16) * 4096;
        x += QString(pass.at(n1 + 1)).toInt(nullptr, 16) * 256;
        x += QString(pass.at(n1 + 2)).toInt(nullptr, 16) * 16;
        x += QString(pass.at(n1 + 3)).toInt(nullptr, 16);
        QChar c(x ^ key.at(n2++).unicode());
        result += c;
        if (n2 >= key.length()) {
            n2 = 0;
        }
    }
    return result;
}

#ifdef HAVE_KEYCHAIN
bool isKeychainEnabled() {
    return !ApplicationInfo::isPortable() &&
           PsiOptions::instance()->getOption("options.keychain.enabled", true).toBool();
}
#endif

QString logencode(QString str) {
    str.replace(QRegExp("\\\\"), "\\\\");  // backslash to double-backslash
    str.replace(QRegExp("\\|"), "\\p");    // pipe to \p
    str.replace(QRegExp("\n"), "\\n");     // newline to \n
    return str;
}

QString logdecode(const QString& str) {
    QString ret;

    for (int n = 0; n < str.length(); ++n) {
        if (str.at(n) == '\\') {
            ++n;
            if (n >= str.length()) {
                break;
            }
            if (str.at(n) == 'n') {
                ret.append('\n');
            }
            if (str.at(n) == 'p') {
                ret.append('|');
            }
            if (str.at(n) == '\\') {
                ret.append('\\');
            }
        } else {
            ret.append(str.at(n));
        }
    }

    return ret;
}

bool fileCopy(const QString& src, const QString& dest) {
    QFile in(src);
    QFile out(dest);

    if (!in.open(QIODevice::ReadOnly)) {
        return false;
    }
    if (!out.open(QIODevice::WriteOnly)) {
        return false;
    }

    char* dat = new char[16384];
    int n = 0;
    while (!in.atEnd()) {
        n = int(in.read(dat, 16384));
        if (n == -1) {
            delete[] dat;
            return false;
        }
        out.write(dat, n);
    }
    delete[] dat;

    out.close();
    in.close();

    return true;
}

/** Detect default player helper on unix like systems
 */
QString soundDetectPlayer() {
    // prefer ALSA on linux
    if (QFile("/proc/asound").exists()) {
        return "aplay -q";
    }
    // fallback to "play"
    return "play";
}

#include <QLayout>
QLayout* rw_recurseFindLayout(QLayout* lo, QWidget* w) {
    // printf("scanning layout: %p\n", lo);
    for (int index = 0; index < lo->count(); ++index) {
        QLayoutItem* i = lo->itemAt(index);
        // printf("found: %p,%p\n", i->layout(), i->widget());
        QLayout* slo = i->layout();
        if (slo) {
            QLayout* tlo = rw_recurseFindLayout(slo, w);
            if (tlo) {
                return tlo;
            }
        } else if (i->widget() == w) {
            return lo;
        }
    }
    return nullptr;
}

QLayout* rw_findLayoutOf(QWidget* w) {
    return rw_recurseFindLayout(w->parentWidget()->layout(), w);
}

void replaceWidget(QWidget* a, QWidget* b) {
    if (!a) {
        return;
    }

    QLayout* lo = rw_findLayoutOf(a);
    if (!lo) {
        return;
    }
    // printf("decided on this: %p\n", lo);

    if (lo->inherits("QBoxLayout")) {
        QBoxLayout* bo = static_cast<QBoxLayout*>(lo);
        int n = bo->indexOf(a);
        bo->insertWidget(n + 1, b);
        delete a;
    }
}

void closeDialogs(QWidget* w) {
    // close qmessagebox?
    QList<QDialog*> dialogs;
    QObjectList list = w->children();
    for (QObjectList::Iterator it = list.begin(); it != list.end(); ++it) {
        if ((*it)->inherits("QDialog")) {
            dialogs.append(static_cast<QDialog*>(*it));
        }
    }
    for (QDialog* w : dialogs) {
        w->close();
    }
}

void reorderGridLayout(QGridLayout* layout, int maxCols) {
    QList<QLayoutItem*> items;
    for (int i = 0; i < layout->rowCount(); i++) {
        for (int j = 0; j < layout->columnCount(); j++) {
            QLayoutItem* item = layout->itemAtPosition(i, j);
            if (item) {
                layout->removeItem(item);
                if (item->isEmpty()) {
                    delete item;
                } else {
                    items.append(item);
                }
            }
        }
    }
    int col = 0, row = 0;
    while (!items.isEmpty()) {
        QLayoutItem* item = items.takeAt(0);
        layout->addItem(item, row, col);
        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }
}

void clearMenu(QMenu* m) {
    m->clear();
    QObjectList l = m->children();
    for (QObject* obj : l) {
        QMenu* child = dynamic_cast<QMenu*>(obj);
        if (child) {
            delete child;
        }
    }
}

bool isKde() {
    return qgetenv("XDG_SESSION_DESKTOP") == "KDE" ||
           qgetenv("DESKTOP_SESSION").endsWith("plasma") ||
           qgetenv("DESKTOP_SESSION").endsWith("plasma5");
}

void bringToFront(QWidget* widget, bool) {
    Q_ASSERT(widget);
    QWidget* w = widget->window();

#ifdef HAVE_X11
    if (QX11Info::isPlatformX11()) {
        // If we're not on the current desktop, do the hide/show trick
        long dsk, curr_dsk;
        Window win = w->winId();
        if (X11WindowSystem::instance()->desktopOfWindow(&win, &dsk) &&
            X11WindowSystem::instance()->currentDesktop(&curr_dsk)) {
            // qDebug() << "bringToFront current desktop=" << curr_dsk << " windowDesktop=" << dsk;
            if ((dsk != curr_dsk) && (dsk != -1)) {  // second condition for sticky windows
                w->hide();
            }
        }
    }

    // FIXME: multi-desktop hacks for Win and Mac required
#endif

    if (w->isMaximized()) {
        w->showMaximized();
    } else {
        w->showNormal();
    }

    // if(grabFocus)
    //    w->setActiveWindow();
    w->raise();
    w->activateWindow();

#if 0
    // hack to real bring to front in kde. kde (at least 4.8.5) forbids stilling
    // focus from other applications. this may be fixed on more recent versions.
    // should be removed some day. preferable way for such hacks is plugins.
    // probably works only with gcc.
    //
    // with kde5 this code just crashes. so should be reimpented as a plugin
    if (isKde()) {
        typedef int (*ActWinFunction)(WId, long);
        ActWinFunction kwinActivateWindow = (ActWinFunction)QLibrary::resolve(
                    "libkdeui", 5, "_ZN13KWindowSystem17forceActiveWindowEml");
        if (kwinActivateWindow) {
            kwinActivateWindow(widget->winId(), 0);
        }
    }
#endif
}

bool operator!=(const QMap<QString, QString>& m1, const QMap<QString, QString>& m2) {
    if (m1.size() != m2.size()) return true;

    QMap<QString, QString>::ConstIterator it = m1.begin(), it2;
    for (; it != m1.end(); ++it) {
        it2 = m2.find(it.key());
        if (it2 == m2.end()) {
            return true;
        }
        if (it.value() != it2.value()) {
            return true;
        }
    }

    return false;
}

//----------------------------------------------------------------------------
// ToolbarPrefs
//----------------------------------------------------------------------------

ToolbarPrefs::ToolbarPrefs()
        : dock(Qt3Dock_Top)
        // , dirty(true)
        , on(false)
        , locked(false)
        // , stretchable(false)
        // , index(0)
        , nl(true)
// , extraOffset(0)
{
    id = QUuid::createUuid().toString();
}

bool ToolbarPrefs::operator==(const ToolbarPrefs& other) {
    return id == other.id && name == other.name && keys == other.keys && dock == other.dock &&
           // dirty == other.dirty &&
           on == other.on && locked == other.locked &&
           // stretchable == other.stretchable &&
           // index == other.index &&
           nl == other.nl;
    // extraOffset == other.extraOffset;
}

int versionStringToInt(const char* version) {
    QString str = QString::fromLatin1(version);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    QStringList parts = str.split('.', Qt::KeepEmptyParts);
#else
    QStringList parts = str.split('.', QString::KeepEmptyParts);
#endif
    if (parts.count() != 3) {
        return 0;
    }

    int versionInt = 0;
    for (int n = 0; n < 3; ++n) {
        bool ok;
        int x = parts[n].toInt(&ok);
        if (ok && x >= 0 && x <= 0xff) {
            versionInt <<= 8;
            versionInt += x;
        } else {
            return 0;
        }
    }
    return versionInt;
}

int qVersionInt() {
    static int out = -1;
    if (out == -1) {
        out = versionStringToInt(qVersion());
    }
    return out;
}
