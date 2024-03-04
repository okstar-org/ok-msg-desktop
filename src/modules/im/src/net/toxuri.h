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


#ifndef TOXURI_H
#define TOXURI_H

#include <QDialog>

bool handleToxURI(const QString& toxURI);

// Internals
class QByteArray;
class QPlainTextEdit;
bool toxURIEventHandler(const QByteArray& eventData);
class ToxURIDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ToxURIDialog(QWidget* parent, const QString& userId, const QString& message);
    QString getRequestMessage();

private:
    QPlainTextEdit* messageEdit;
};

#endif // TOXURI_H
