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

#ifndef PSIICONSET_H
#define PSIICONSET_H

#include "iconset.h"

#include <QMap>

class UserListItem;

namespace XMPP {
class Jid;
class Status;
}  // namespace XMPP

class PsiIconset : public QObject {
    Q_OBJECT
public:
    static PsiIconset* instance();

    bool loadSystem();
    void reloadRoster();
    bool loadAll();

    QHash<QString, Iconset*> roster;
    QList<Iconset*> emoticons;
    Iconset moods;
    Iconset activities;
    Iconset clients;
    Iconset affiliations;
    const Iconset& system() const;
    void stripFirstAnimFrame(Iconset*);
    static void removeAnimation(Iconset*);

    // these two can possibly fail (and return 0)
    PsiIcon* statusPtr(int);
    PsiIcon* statusPtr(const XMPP::Status&);

    // these two return empty PsiIcon on failure and are safe
    PsiIcon status(int);
    PsiIcon status(const XMPP::Status&);

    // JID-enabled status functions
    PsiIcon* statusPtr(const XMPP::Jid&, int);
    PsiIcon* statusPtr(const XMPP::Jid&, const XMPP::Status&);

    PsiIcon status(const XMPP::Jid&, int);
    PsiIcon status(const XMPP::Jid&, const XMPP::Status&);

    // functions to get status icon by transport name
    PsiIcon* transportStatusPtr(QString name, int);
    PsiIcon* transportStatusPtr(QString name, const XMPP::Status&);

    PsiIcon transportStatus(QString name, int);
    PsiIcon transportStatus(QString name, const XMPP::Status&);

    PsiIcon* statusPtr(UserListItem*);
    PsiIcon status(UserListItem*);

    QString caps2client(const QString& name);
signals:
    void emoticonsChanged();
    void systemIconsSizeChanged(int);
    void rosterIconsSizeChanged(int);

public slots:
    static void reset();

private slots:
    void optionChanged(const QString& option);

private:
    PsiIconset();
    ~PsiIconset();

    class Private;
    Private* d;

    static PsiIconset* instance_;

    bool loadRoster();
    void loadEmoticons();
    bool loadMoods();
    bool loadActivity();
    bool loadClients();
    bool loadAffiliations();
    void loadStatusIconDefinitions();
};

QString status2name(int s);

#endif  // PSIICONSET_H
