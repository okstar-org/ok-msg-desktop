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

#ifndef SEARCHSETTINGSFORM_H
#define SEARCHSETTINGSFORM_H

#include <QWidget>
#include "src/widget/searchtypes.h"

namespace Ui {
class SearchSettingsForm;
}

class SearchSettingsForm : public QWidget {
    Q_OBJECT

public:
    explicit SearchSettingsForm(QWidget* parent = nullptr);
    ~SearchSettingsForm();

    ParameterSearch getParameterSearch();
    void reloadTheme();

private:
    Ui::SearchSettingsForm* ui;
    QDate startDate;
    bool isUpdate{false};

    void updateStartDateLabel();
    void setUpdate(const bool isUpdate);

private slots:
    void onStartSearchSelected(const int index);
    void onRegisterClicked(const bool checked);
    void onWordsOnlyClicked(const bool checked);
    void onRegularClicked(const bool checked);
    void onChoiceDate();

signals:
    void updateSettings(const bool isUpdate);
};

#endif  // SEARCHSETTINGSFORM_H
