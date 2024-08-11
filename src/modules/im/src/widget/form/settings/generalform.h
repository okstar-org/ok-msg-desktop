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

#ifndef GENERALFORM_H
#define GENERALFORM_H

#include "genericsettings.h"
#include "src/widget/form/settingswidget.h"

namespace Ui {
class GeneralSettings;
}

class SettingsWidget;

class GeneralForm : public GenericForm {
    Q_OBJECT
public:
    explicit GeneralForm(SettingsWidget* parent);
    ~GeneralForm();
    virtual QString getFormName() final override { return tr("General"); }
signals:
    void updateIcons();

private slots:
    //
    void on_smileyPackBrowser_currentIndexChanged(int index);
    //void on_emoticonSize_editingFinished();
    //void on_styleBrowser_currentIndexChanged(QString style);
   // void on_timestamp_editTextChanged(const QString& format);
    //void on_dateFormats_editTextChanged(const QString& format);
    void on_textStyleComboBox_currentTextChanged();
    void on_useEmoticons_stateChanged();
    void on_notify_stateChanged();
    void on_desktopNotify_stateChanged();
   // void on_notifySound_stateChanged();
   // void on_notifyHide_stateChanged(int);
    void on_busySound_stateChanged();
    void on_showWindow_stateChanged();
    void on_groupOnlyNotfiyWhenMentioned_stateChanged();



    //void on_themeColorCBox_currentIndexChanged(int);

    //void on_txtChatFont_currentFontChanged(const QFont& f);
   // void on_txtChatFontSize_valueChanged(int arg1);
   // void on_useNameColors_stateChanged(int value);

private:
    //void retranslateUi();

    //
    void retranslateUi();
    void reloadSmileys();

private:
    Ui::GeneralSettings* bodyUI;
    SettingsWidget* parent;

    QList<QLabel*> smileLabels;
    QList<std::shared_ptr<QIcon>> emoticonsIcons;
    const int MAX_FORMAT_LENGTH = 128;
};

#endif
