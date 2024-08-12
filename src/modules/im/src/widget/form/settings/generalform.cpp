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

#include "generalform.h"
#include "ui_generalsettings.h"

#include <QFileDialog>
#include <QDesktopWidget>

#include "base/OkSettings.h"
#include "lib/settings/translator.h"
#include "src/base/RecursiveSignalBlocker.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/lib/settings/style.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/persistence/smileypack.h"
#include "src/widget/form/settingswidget.h"

#include "src/widget/widget.h"

/**
 * @class GeneralForm
 *
 * This form contains all settings that are not suited to other forms
 */
GeneralForm::GeneralForm(SettingsWidget* myParent)
        : GenericForm(QPixmap(":/img/settings/general.png")), bodyUI(new Ui::GeneralSettings) {
    parent = myParent;

    bodyUI->setupUi(this);

    // block all child signals during initialization
    const ok::base::RecursiveSignalBlocker signalBlocker(this);

    Settings& s = Settings::getInstance();
    //先获取当前语言
    auto & okSettings = ok::base::OkSettings::getInstance();
    const QFont chatBaseFont = s.getChatMessageFont();
    bodyUI->txtChatFontSize->setValue(QFontInfo(chatBaseFont).pixelSize());
    bodyUI->txtChatFont->setCurrentFont(chatBaseFont);
    int index = static_cast<int>(s.getStylePreference());
    bodyUI->textStyleComboBox->setCurrentIndex(index);
    bodyUI->useNameColors->setChecked(s.getEnableGroupChatsColor());

    bodyUI->notify->setChecked(s.getNotify());
    // Note: UI is boolean inversed from settings to maintain setting file backwards compatibility
    bodyUI->groupOnlyNotfiyWhenMentioned->setChecked(!s.getGroupAlwaysNotify());
    bodyUI->groupOnlyNotfiyWhenMentioned->setEnabled(s.getNotify());
    bodyUI->notifySound->setChecked(s.getNotifySound());
    bodyUI->notifyHide->setChecked(s.getNotifyHide());
    bodyUI->notifySound->setEnabled(s.getNotify());
    bodyUI->busySound->setChecked(s.getBusySound());
    bodyUI->busySound->setEnabled(s.getNotifySound() && s.getNotify());
#if DESKTOP_NOTIFICATIONS
    bodyUI->desktopNotify->setChecked(s.getDesktopNotify());
    bodyUI->desktopNotify->setEnabled(s.getNotify());
#else
    bodyUI->desktopNotify->hide();
#endif

    bodyUI->showWindow->setChecked(s.getShowWindow());

//    bodyUI->cbGroupchatPosition->setChecked(s.getGroupchatPosition());
//    bodyUI->cbCompactLayout->setChecked(s.getCompactLayout());
//    bodyUI->cbSeparateWindow->setChecked(s.getSeparateWindow());
//    bodyUI->cbDontGroupWindows->setChecked(s.getDontGroupWindows());
//    bodyUI->cbDontGroupWindows->setEnabled(s.getSeparateWindow());
//    bodyUI->cbShowIdenticons->setChecked(s.getShowIdenticons());

    bodyUI->useEmoticons->setChecked(s.getUseEmoticons());
    for (auto entry : SmileyPack::listSmileyPacks())
        bodyUI->smileyPackBrowser->addItem(entry.first, entry.second);

    smileLabels = {bodyUI->smile1, bodyUI->smile2, bodyUI->smile3, bodyUI->smile4, bodyUI->smile5};

    int currentPack = bodyUI->smileyPackBrowser->findData(s.getSmileyPack());
    bodyUI->smileyPackBrowser->setCurrentIndex(currentPack);
    reloadSmileys();
    bodyUI->smileyPackBrowser->setEnabled(bodyUI->useEmoticons->isChecked());

  /*
    bodyUI->styleBrowser->addItem(tr("None"));
    bodyUI->styleBrowser->addItems(QStyleFactory::keys());

    QString style;
    if (QStyleFactory::keys().contains(s.getStyle()))
        style = s.getStyle();
    else
        style = tr("None");

    bodyUI->styleBrowser->setCurrentText(style);

    for (QString color : Style::getThemeColorNames())
        bodyUI->themeColorCBox->addItem(color);

    bodyUI->themeColorCBox->setCurrentIndex(s.getThemeColor());
    */
    bodyUI->emoticonSize->setValue(s.getEmojiFontPointSize());


#ifndef QTOX_PLATFORM_EXT
   // bodyUI->autoAwayLabel->setEnabled(
    //        false);  // these don't seem to change the appearance of the widgets,
   // bodyUI->autoAwaySpinBox->setEnabled(false);  // though they are unusable
#endif

    eventsInit();
    settings::Translator::registerHandler(std::bind(&GeneralForm::retranslateUi, this), this);
}

GeneralForm::~GeneralForm() {
    settings::Translator::unregister(this);
    delete bodyUI;
}

//
/*
void GeneralForm::on_styleBrowser_currentIndexChanged(QString style)
{
    if (bodyUI->styleBrowser->currentIndex() == 0)
        Settings::getInstance().setStyle("None");
    else
        Settings::getInstance().setStyle(style);

    this->setStyle(QStyleFactory::create(style));
    parent->setBodyHeadStyle(style);
}
*/

/*
void GeneralForm::on_emoticonSize_editingFinished()
{
    Settings::getInstance().setEmojiFontPointSize(bodyUI->emoticonSize->value());
}
*/

void GeneralForm::on_useEmoticons_stateChanged()
{
    Settings::getInstance().setUseEmoticons(bodyUI->useEmoticons->isChecked());
    bodyUI->smileyPackBrowser->setEnabled(bodyUI->useEmoticons->isChecked());
}

void GeneralForm::on_textStyleComboBox_currentTextChanged()
{
    Settings::StyleType styleType =
        static_cast<Settings::StyleType>(bodyUI->textStyleComboBox->currentIndex());
    Settings::getInstance().setStylePreference(styleType);
}

void GeneralForm::on_smileyPackBrowser_currentIndexChanged(int index)
{
    QString filename = bodyUI->smileyPackBrowser->itemData(index).toString();
    Settings::getInstance().setSmileyPack(filename);
    reloadSmileys();
}

/**
 * @brief Reload smileys and size information.
 */
void GeneralForm::reloadSmileys()
{
    QList<QStringList> emoticons = SmileyPack::getInstance().getEmoticons();

    // sometimes there are no emoticons available, don't crash in this case
    if (emoticons.isEmpty()) {
        qDebug() << "reloadSmilies: No emoticons found";
        return;
    }

    QStringList smileys;
    for (int i = 0; i < emoticons.size(); ++i)
        smileys.push_front(emoticons.at(i).first());

    emoticonsIcons.clear();
    const QSize size(18, 18);
    for (int i = 0; i < smileLabels.size(); ++i) {
        std::shared_ptr<QIcon> icon = SmileyPack::getInstance().getAsIcon(smileys[i]);
        emoticonsIcons.append(icon);
        smileLabels[i]->setPixmap(icon->pixmap(size));
        smileLabels[i]->setToolTip(smileys[i]);
    }

    // set maximum size of emoji
    QDesktopWidget desktop;
    // 8 is the count of row and column in emoji's in widget
    const int sideSize = 8;
    int maxSide = qMin(desktop.geometry().height() / sideSize, desktop.geometry().width() / sideSize);
    QSize maxSize(maxSide, maxSide);

    QSize actualSize = emoticonsIcons.first()->actualSize(maxSize);
    bodyUI->emoticonSize->setMaximum(actualSize.width());
}

void GeneralForm::on_notify_stateChanged()
{
    const bool notify = bodyUI->notify->isChecked();
    Settings::getInstance().setNotify(notify);
    bodyUI->groupOnlyNotfiyWhenMentioned->setEnabled(notify);
    bodyUI->notifySound->setEnabled(notify);
    bodyUI->busySound->setEnabled(notify && bodyUI->notifySound->isChecked());
    bodyUI->desktopNotify->setEnabled(notify);
}
/*
void GeneralForm::on_notifySound_stateChanged()
{
    const bool notify = bodyUI->notifySound->isChecked();
    Settings::getInstance().setNotifySound(notify);
    bodyUI->busySound->setEnabled(notify);
}
*/
void GeneralForm::on_desktopNotify_stateChanged()
{
    const bool notify = bodyUI->desktopNotify->isChecked();
    Settings::getInstance().setDesktopNotify(notify);
}

void GeneralForm::on_busySound_stateChanged()
{
    Settings::getInstance().setBusySound(bodyUI->busySound->isChecked());
}

void GeneralForm::on_showWindow_stateChanged()
{
    Settings::getInstance().setShowWindow(bodyUI->showWindow->isChecked());
}

void GeneralForm::on_groupOnlyNotfiyWhenMentioned_stateChanged()
{
    // Note: UI is boolean inversed from settings to maintain setting file backwards compatibility
    Settings::getInstance().setGroupAlwaysNotify(!bodyUI->groupOnlyNotfiyWhenMentioned->isChecked());
}

/*
void GeneralForm::on_themeColorCBox_currentIndexChanged(int)
{
    int index = bodyUI->themeColorCBox->currentIndex();
    Settings::getInstance().setThemeColor(index);
    Style::setThemeColor(index);
    Style::applyTheme();
}*/

/**
 * @brief Retranslate all elements in the form.
 */

void GeneralForm::retranslateUi() { bodyUI->retranslateUi(this); }

