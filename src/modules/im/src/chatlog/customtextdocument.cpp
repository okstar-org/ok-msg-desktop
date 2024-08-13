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

#include "customtextdocument.h"
#include "src/lib/settings/style.h"
#include "src/persistence/settings.h"
#include "src/persistence/smileypack.h"

#include <QDebug>
#include <QIcon>
#include <QUrl>

CustomTextDocument::CustomTextDocument(QObject* parent) : QTextDocument(parent) {
    setUndoRedoEnabled(false);
    setUseDesignMetrics(false);
}

QVariant CustomTextDocument::loadResource(int type, const QUrl& name) {
    if (type == QTextDocument::ImageResource && name.scheme() == "key") {
        QSize size = QSize(Settings::getInstance().getEmojiFontPointSize(),
                           Settings::getInstance().getEmojiFontPointSize());
        QString fileName = QUrl::fromPercentEncoding(name.toEncoded()).mid(4).toHtmlEscaped();

        std::shared_ptr<QIcon> icon = SmileyPack::getInstance().getAsIcon(fileName);
        emoticonIcons.append(icon);
        return icon->pixmap(size);
    }

    return QTextDocument::loadResource(type, name);
}
