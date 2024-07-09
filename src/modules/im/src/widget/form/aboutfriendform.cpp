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

#include "aboutfriendform.h"
#include "src/core/core.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/widget/gui.h"
#include "src/widget/style.h"
#include "src/widget/widget.h"
#include "ui_aboutfriendform.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPainter>
#include <QSvgRenderer>

#include <base/SvgUtils.h>

AboutFriendForm::AboutFriendForm(std::unique_ptr<IAboutFriend> _about, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::AboutFriendForm)
    , about{std::move(_about)}
    , widget{Widget::getInstance()}
    , profile{Nexus::getInstance().getProfile()}
{
    ui->setupUi(this);

    setAttribute(Qt::WA_StyledBackground);
    reloadTheme();

    connect(ui->sendMessage, &QPushButton::clicked, this, &AboutFriendForm::onSendMessageClicked);
    connect(ui->autoacceptfile, &QCheckBox::clicked, this, &AboutFriendForm::onAutoAcceptDirClicked);
    connect(ui->autoacceptcall, SIGNAL(activated(int)), this, SLOT(onAutoAcceptCallClicked(void)));

    connect(ui->selectSaveDir, &QPushButton::clicked, this, &AboutFriendForm::onSelectDirClicked);
    connect(ui->removeHistory, &QPushButton::clicked, this, &AboutFriendForm::onRemoveHistoryClicked);
    about->connectTo_autoAcceptDirChanged(this, [this](const QString& dir){ onAutoAcceptDirChanged(dir); });

    const QString dir = about->getAutoAcceptDir();
    ui->autoacceptfile->setChecked(!dir.isEmpty());

//    ui->removeHistory->setEnabled(about->isHistoryExistence());

    const int index = static_cast<int>(about->getAutoAcceptCall());
    ui->autoacceptcall->setCurrentIndex(index);

    ui->selectSaveDir->setEnabled(ui->autoacceptfile->isChecked());

    if (ui->autoacceptfile->isChecked()) {
        ui->selectSaveDir->setText(about->getAutoAcceptDir());
    }


    ui->id->setText(about->getPublicKey().toString());
    ui->statusMessage->setText(about->getStatusMessage());
    ui->avatar->setPixmap(about->getAvatar());

    ui->userName->setText(about->getName());
    connect(about->getFriend(), &Friend::nameChanged, [&](auto name){
        ui->userName->setText(name);
        ui->alias->setPlaceholderText(name);
    });

    ui->alias->setText(about->getAlias());
    ui->alias->setPlaceholderText(about->getName());
    connect(about->getFriend(), &Contact::aliasChanged, [&](auto alias){
        ui->alias->setText(alias);
    });

    connect(ui->alias, &QLineEdit::textChanged, this, &AboutFriendForm::onAliasChanged);

    connect(&GUI::getInstance(), &GUI::themeApplyRequest, this, &AboutFriendForm::reloadTheme);
}

static QString getAutoAcceptDir(const QString& dir)
{
    //: popup title
    const QString title = AboutFriendForm::tr("Choose an auto accept directory");
    return QFileDialog::getExistingDirectory(Q_NULLPTR, title, dir);
}

void AboutFriendForm::onAutoAcceptDirClicked()
{
    const QString dir = [&]{
        if (!ui->autoacceptfile->isChecked()) {
            return QString{};
        }

        return getAutoAcceptDir(about->getAutoAcceptDir());
    }();

    about->setAutoAcceptDir(dir);
}

void AboutFriendForm::onAutoAcceptDirChanged(const QString& path)
{
    const bool enabled = !path.isNull();
    ui->autoacceptfile->setChecked(enabled);
    ui->selectSaveDir->setEnabled(enabled);
    ui->selectSaveDir->setText(enabled ? path : tr("Auto accept for this contact is disabled"));
}


void AboutFriendForm::onAutoAcceptCallClicked()
{
    const int index = ui->autoacceptcall->currentIndex();
    const IFriendSettings::AutoAcceptCallFlags flag{index};
    about->setAutoAcceptCall(flag);
}

void AboutFriendForm::onSelectDirClicked()
{
    const QString dir = getAutoAcceptDir(about->getAutoAcceptDir());
    about->setAutoAcceptDir(dir);
}

/**
 * @brief Called when user clicks the bottom OK button, save all settings
 */
void AboutFriendForm::onSendMessageClicked()
{
    auto w = Widget::getInstance();
    emit w->toSendMessage(ui->id->text());
}

void AboutFriendForm::onRemoveHistoryClicked()
{
   const bool retYes = GUI::askQuestion(tr("Confirmation"),
                                   tr("Are you sure to remove %1 chat history?").arg(about->getName()),
                                   /* defaultAns = */ false, /* warning = */ true, /* yesno = */ true);
    if (!retYes) {
        return;
    }


    auto w = Widget::getInstance();
    emit w->toClearHistory(ui->id->text());

//   const bool result = about->clearHistory();
//    if (!result) {
//        GUI::showWarning(tr("History removed"),
//                         tr("Failed to remove chat history with %1!").arg(about->getName()).toHtmlEscaped());
//        return;
//    }
//    emit histroyRemoved();
//    ui->removeHistory->setEnabled(false); // For know clearly to has removed the history
}

AboutFriendForm::~AboutFriendForm()
{
    qDebug() << __func__;
    delete ui;
}

void AboutFriendForm::setName(const QString &name)
{
    ui->userName->setText(name);
}

void AboutFriendForm::reloadTheme()
{
    setStyleSheet(Style::getStylesheet("window/general.css"));
}

void AboutFriendForm::onAliasChanged(const QString &text)
{

    auto fid = ui->id->text();

    auto f = FriendList::findFriend(ContactId(fid));
    f->setAlias(text);

    Core::getInstance()->setFriendAlias(fid, text);
}
