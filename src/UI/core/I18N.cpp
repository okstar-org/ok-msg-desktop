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
#include "I18N.h"

#include <QCoreApplication>
#include <QDebug>
#include <QLibraryInfo>
#include <QLocale>
#include <QTextCodec>

#include <base/logs.h>

namespace core {

const QString APPLICATION_ENCODING = "UTF-8";
const QString TRANSLATIONS_PATH = "translations";

I18N::I18N(const QString& path, const QString& encoding) {
    m_path = path;
    m_encoding = encoding;
}

void I18N::Init() {
    I18N* i18n = new I18N(TRANSLATIONS_PATH, APPLICATION_ENCODING);
    i18n->translate();

    self = i18n;
}

QString I18N::getEncoding() const { return m_encoding; }

void I18N::setEncoding(const QString& encoding) { m_encoding = encoding; }

QString I18N::getPath() const { return m_path; }

void I18N::setPath(const QString& path) { m_path = path; }

void I18N::translate() {
    QLocale::setDefault(QLocale::Chinese);

    QTranslator m_apTranslator, m_qtTranslator;
    m_qtTranslator.load("qt_" + QLocale::system().name(),
                        QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    qApp->installTranslator(&m_qtTranslator);

    QString classroomQM(":/" + m_path + "/classroom_zh.qm");

    qDebug() << "load qm file:" << classroomQM;

    m_apTranslator.load(classroomQM);
    qApp->installTranslator(&m_apTranslator);
}

}  // namespace core
