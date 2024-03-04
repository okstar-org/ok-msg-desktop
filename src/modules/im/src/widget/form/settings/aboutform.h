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

#ifndef ABOUTFORM_H
#define ABOUTFORM_H

#include "genericsettings.h"

#include <memory>
class Core;
class QTimer;
class QString;
class UpdateCheck;
class QLayoutItem;

namespace Ui {
class AboutSettings;
}

class AboutForm : public GenericForm
{
    Q_OBJECT
public:
    AboutForm(UpdateCheck* updateCheck);
    ~AboutForm();
    virtual QString getFormName() final override
    {
        return tr("About");
    }

public slots:
    void onUpdateAvailable(QString latestVersion, QUrl link);
    void onUpToDate();
    void onUpdateCheckFailed();

private:
    void retranslateUi();
    void replaceVersions();
    inline QString createLink(QString path, QString text) const;

private:
    Ui::AboutSettings* bodyUI;
    QTimer* progressTimer;
    UpdateCheck* updateCheck;
    QMetaObject::Connection linkConnection;
};

#endif // ABOUTFORM_H
