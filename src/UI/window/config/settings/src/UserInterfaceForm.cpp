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

#include "UserInterfaceForm.h"
#include "base/RecursiveSignalBlocker.h"
#include "ui_UserInterfaceForm.h"

#include <QDebug>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFont>
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QStyleFactory>
#include <QTime>
#include <QVector>

#include "lib/settings/translator.h"
#include "src/lib/settings/settings.h"
#include "src/lib/settings/style.h"

namespace UI {

/**
 * @class UserInterfaceForm
 *
 * This form contains all settings which change the UI appearance or behaviour.
 * It also contains the smiley configuration.
 */

/**
 * @brief Constructor of UserInterfaceForm.
 * @param myParent Setting widget which will contain this form as tab.
 *
 * Restores all controls from the settings.
 */
UserInterfaceForm::UserInterfaceForm(SettingsWidget* myParent)
        : GenericForm(QPixmap(":/img/settings/general.png"), myParent)
        , bodyUI{new Ui::UserInterfaceForm} {
    bodyUI->setupUi(this);
    parent = myParent;

    // block all child signals during initialization
    const RecursiveSignalBlocker signalBlocker(this);

//    Settings& s = Settings::getInstance();
//    const QFont chatBaseFont = s.getChatMessageFont();
//    bodyUI->txtChatFontSize->setValue(QFontInfo(chatBaseFont).pixelSize());
//    bodyUI->txtChatFont->setCurrentFont(chatBaseFont);
//    int index = static_cast<int>(s.getStylePreference());
//    bodyUI->textStyleComboBox->setCurrentIndex(index);
//    bodyUI->useNameColors->setChecked(s.getEnableGroupChatsColor());
//
//    bodyUI->notify->setChecked(s.getNotify());
// Note: UI is boolean inversed from settings to maintain setting file backwards compatibility
//    bodyUI->groupOnlyNotfiyWhenMentioned->setChecked(!s.getGroupAlwaysNotify());
//    bodyUI->groupOnlyNotfiyWhenMentioned->setEnabled(s.getNotify());
//    bodyUI->notifySound->setChecked(s.getNotifySound());
//    bodyUI->notifyHide->setChecked(s.getNotifyHide());
//    bodyUI->notifySound->setEnabled(s.getNotify());
//    bodyUI->busySound->setChecked(s.getBusySound());
//    bodyUI->busySound->setEnabled(s.getNotifySound() && s.getNotify());
#if DESKTOP_NOTIFICATIONS
    bodyUI->desktopNotify->setChecked(s.getDesktopNotify());
    bodyUI->desktopNotify->setEnabled(s.getNotify());
#else
    bodyUI->desktopNotify->hide();
#endif

    //    bodyUI->showWindow->setChecked(s.getShowWindow());

    //    bodyUI->cbGroupchatPosition->setChecked(s.getGroupchatPosition());
    //    bodyUI->cbCompactLayout->setChecked(s.getCompactLayout());
    //    bodyUI->cbSeparateWindow->setChecked(s.getSeparateWindow());
    //    bodyUI->cbDontGroupWindows->setChecked(s.getDontGroupWindows());
    //    bodyUI->cbDontGroupWindows->setEnabled(s.getSeparateWindow());
    //    bodyUI->cbShowIdenticons->setChecked(s.getShowIdenticons());

    //    bodyUI->useEmoticons->setChecked(s.getUseEmoticons());
    //    for (auto entry : SmileyPack::listSmileyPacks())
    //        bodyUI->smileyPackBrowser->addItem(entry.first, entry.second);

    bodyUI->styleBrowser->addItem(tr("None"));
    bodyUI->styleBrowser->addItems(QStyleFactory::keys());

    //    QString style;
    //    if (QStyleFactory::keys().contains(s.getStyle()))
    //        style = s.getStyle();
    //    else
    //        style = tr("None");

    //    bodyUI->styleBrowser->setCurrentText(style);

    //    for (QString color : Style::getThemeColorNames())
    //        bodyUI->themeColorCBox->addItem(color);

    //    bodyUI->themeColorCBox->setCurrentIndex(s.getThemeColor());
    //    bodyUI->emoticonSize->setValue(s.getEmojiFontPointSize());

    QLocale ql;
    QStringList timeFormats;
    timeFormats << ql.timeFormat(QLocale::ShortFormat) << ql.timeFormat(QLocale::LongFormat)
                << "hh:mm AP" << "hh:mm:ss AP" << "hh:mm:ss";
    timeFormats.removeDuplicates();
    bodyUI->timestamp->addItems(timeFormats);

    QRegularExpression re(QString("^[^\\n]{0,%0}$").arg(MAX_FORMAT_LENGTH));
    QRegularExpressionValidator* validator = new QRegularExpressionValidator(re, this);
    //    QString timeFormat = s.getTimestampFormat();

    //    if (!re.match(timeFormat).hasMatch())
    //        timeFormat = timeFormats[0];
    //
    //    bodyUI->timestamp->setCurrentText(timeFormat);
    //    bodyUI->timestamp->setValidator(validator);
    //    on_timestamp_editTextChanged(timeFormat);
    //
    //    QStringList dateFormats;
    //    dateFormats << QStringLiteral("yyyy-MM-dd") // ISO 8601
    //                                                 format strings from system locale
    //                << ql.dateFormat(QLocale::LongFormat) << ql.dateFormat(QLocale::ShortFormat)
    //                << ql.dateFormat(QLocale::NarrowFormat) << "dd-MM-yyyy"
    //                << "d-MM-yyyy"
    //                << "dddd dd-MM-yyyy"
    //                << "dddd d-MM";

    //    dateFormats.removeDuplicates();
    //    bodyUI->dateFormats->addItems(dateFormats);
    //
    //    QString dateFormat = s.getDateFormat();
    //    if (!re.match(dateFormat).hasMatch())
    //        dateFormat = dateFormats[0];
    //
    //    bodyUI->dateFormats->setCurrentText(dateFormat);
    //    bodyUI->dateFormats->setValidator(validator);
    //    on_dateFormats_editTextChanged(dateFormat);

    //    eventsInit();
    //    settings::Translator::registerHandler(std::bind(&UserInterfaceForm::retranslateUi, this),
    //    this);
}

UserInterfaceForm::~UserInterfaceForm() {
    settings::Translator::unregister(this);
    delete bodyUI;
}

// void UserInterfaceForm::on_styleBrowser_currentIndexChanged(QString style)
//{
//     if (bodyUI->styleBrowser->currentIndex() == 0)
//         Settings::getInstance().setStyle("None");
//     else
//         Settings::getInstance().setStyle(style);
//
//     this->setStyle(QStyleFactory::create(style));
//     parent->setBodyHeadStyle(style);
// }

void UserInterfaceForm::on_timestamp_editTextChanged(const QString& format) {
    QString timeExample = QTime::currentTime().toString(format);
    bodyUI->timeExample->setText(timeExample);

    //    Settings::getInstance().setTimestampFormat(format);
    //    QString locale = Settings::getInstance().getTranslation();
    //    settings::Translator::translate(OK_UIWindowConfig_MODULE, locale);
}

void UserInterfaceForm::on_dateFormats_editTextChanged(const QString& format) {
    QString dateExample = QDate::currentDate().toString(format);
    bodyUI->dateExample->setText(dateExample);

    //    Settings::getInstance().setDateFormat(format);
    //    QString locale = Settings::getInstance().getTranslation();
    //    settings::Translator::translate(OK_UIWindowConfig_MODULE, locale);
}

// void UserInterfaceForm::on_useEmoticons_stateChanged()
//{
//     Settings::getInstance().setUseEmoticons(bodyUI->useEmoticons->isChecked());
//     bodyUI->smileyPackBrowser->setEnabled(bodyUI->useEmoticons->isChecked());
// }
//
// void UserInterfaceForm::on_textStyleComboBox_currentTextChanged()
//{
//     Settings::StyleType styleType =
//         static_cast<Settings::StyleType>(bodyUI->textStyleComboBox->currentIndex());
//     Settings::getInstance().setStylePreference(styleType);
// }

// void UserInterfaceForm::on_notify_stateChanged()
//{
//     const bool notify = bodyUI->notify->isChecked();
//     Settings::getInstance().setNotify(notify);
//     bodyUI->groupOnlyNotfiyWhenMentioned->setEnabled(notify);
//     bodyUI->notifySound->setEnabled(notify);
//     bodyUI->busySound->setEnabled(notify && bodyUI->notifySound->isChecked());
//     bodyUI->desktopNotify->setEnabled(notify);
// }

// void UserInterfaceForm::on_notifySound_stateChanged()
//{
//     const bool notify = bodyUI->notifySound->isChecked();
//     Settings::getInstance().setNotifySound(notify);
//     bodyUI->busySound->setEnabled(notify);
// }

// void UserInterfaceForm::on_desktopNotify_stateChanged()
//{
//     const bool notify = bodyUI->desktopNotify->isChecked();
//     Settings::getInstance().setDesktopNotify(notify);
// }
//
// void UserInterfaceForm::on_busySound_stateChanged()
//{
//     Settings::getInstance().setBusySound(bodyUI->busySound->isChecked());
// }
//
// void UserInterfaceForm::on_showWindow_stateChanged()
//{
//     Settings::getInstance().setShowWindow(bodyUI->showWindow->isChecked());
// }
//
// void UserInterfaceForm::on_groupOnlyNotfiyWhenMentioned_stateChanged()
//{
//     // Note: UI is boolean inversed from settings to maintain setting file backwards
//     compatibility
////
/// Settings::getInstance().setGroupAlwaysNotify(!bodyUI->groupOnlyNotfiyWhenMentioned->isChecked());
//}

// void UserInterfaceForm::on_themeColorCBox_currentIndexChanged(int)
//{
//     int index = bodyUI->themeColorCBox->currentIndex();
//     Settings::getInstance().setThemeColor(index);
//     Style::setThemeColor(index);
//     Style::applyTheme();
// }

/**
 * @brief Retranslate all elements in the form.
 */
void UserInterfaceForm::retranslateUi() {
    // Block signals during translation to prevent settings change
    RecursiveSignalBlocker signalBlocker{this};

    bodyUI->retranslateUi(this);

    // Restore text style index once translation is complete
    //    bodyUI->textStyleComboBox->setCurrentIndex(
    //        static_cast<int>(Settings::getInstance().getStylePreference()));
    //
    //    QStringList colorThemes(Style::getThemeColorNames());
    //    for (int i = 0; i < colorThemes.size(); ++i) {
    //        bodyUI->themeColorCBox->setItemText(i, colorThemes[i]);
    //    }
    //
    //    bodyUI->styleBrowser->setItemText(0, tr("None"));
}
//
// void UserInterfaceForm::on_txtChatFont_currentFontChanged(const QFont& f)
//{
//    QFont tmpFont = f;
//    const int px = bodyUI->txtChatFontSize->value();
//
//    if (QFontInfo(tmpFont).pixelSize() != px)
//        tmpFont.setPixelSize(px);
//
//    Settings::getInstance().setChatMessageFont(tmpFont);
//}
//
// void UserInterfaceForm::on_txtChatFontSize_valueChanged(int px)
//{
//    Settings& s = Settings::getInstance();
//    QFont tmpFont = s.getChatMessageFont();
//    const int fontSize = QFontInfo(tmpFont).pixelSize();
//
//    if (px != fontSize) {
//        tmpFont.setPixelSize(px);
//        s.setChatMessageFont(tmpFont);
//    }
//}
//
// void UserInterfaceForm::on_useNameColors_stateChanged(int value)
//{
//    Settings::getInstance().setEnableGroupChatsColor(value);
//}
//
// void UserInterfaceForm::on_notifyHide_stateChanged(int value)
//{
//    Settings::getInstance().setNotifyHide(value);
//}

}  // namespace UI
