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

#ifndef TABCOMPLETER_H
#define TABCOMPLETER_H

#include <QMap>
#include <QString>
#include "src/model/group.h"
#include "src/widget/tool/chattextedit.h"

class TabCompleter : public QObject {
    Q_OBJECT
public:
    TabCompleter(ChatTextEdit* msgEdit, const Group* group);

public slots:
    void complete();
    void reset();

private:
    struct SortableString {
        explicit SortableString(const QString& n) : contents{n} {}
        bool operator<(const SortableString& other) const;
        QString contents;
    };

    ChatTextEdit* msgEdit;
    const Group* group;
    bool enabled;
    const static QString nickSuffix;

    QMap<SortableString, QString> completionMap;
    QMap<SortableString, QString>::Iterator nextCompletion;
    int lastCompletionLength;

    void buildCompletionList();
};

#endif
