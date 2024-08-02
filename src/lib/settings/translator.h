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

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QMap>
#include <QMutex>
#include <QPair>
#include <QVector>
#include <functional>

class QTranslator;

namespace settings {

using Callback = QPair<void*, std::function<void()>>;
static QMutex lock;

class Translator {
public:
    static void translate(const QString& moduleName, const QString& localeName);
    static void registerHandler(const std::function<void()>&, void* owner);
    static void unregister(void* owner);

private:
    //  static QVector<Callback> callbacks;

    //  static QTranslator *translator;
    //  static bool m_loadedQtTranslations;
};
}  // namespace settings

#endif  // TRANSLATOR_H
