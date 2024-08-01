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

#ifndef I18N_H
#define I18N_H

#include <QTranslator>

namespace core {

class I18N;

static I18N* self = nullptr;

class I18N {
public:
    I18N(const QString& path, const QString& encoding);

    static void Init();

    QString getPath() const;
    void setPath(const QString& path);

    QString getEncoding() const;
    void setEncoding(const QString& encoding);

    void translate();

private:
    QString m_path, m_encoding;
};
}  // namespace core

#endif  // I18N_H
