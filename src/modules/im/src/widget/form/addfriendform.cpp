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

#include "addfriendform.h"
#include <QApplication>
#include <QClipboard>
#include <QErrorMessage>
#include <QFileDialog>
#include <QFont>
#include <QMessageBox>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSignalMapper>
#include <QTabWidget>
#include <QWindow>
#include "application.h"
#include "friendform.h"
#include "lib/backend/UserService.h"
#include "lib/session/AuthSession.h"
#include "lib/settings/translator.h"
#include "src/core/core.h"
#include "src/lib/settings/style.h"
#include "src/nexus.h"
#include "src/persistence/settings.h"
#include "src/widget/contentlayout.h"
#include "src/widget/gui.h"
#include "src/widget/tool/croppinglabel.h"
#include "ui_addfriendform.h"

namespace {

QString getToxId(const QString& id) { return id.trimmed(); }

}  // namespace

/**
 * @var QString AddFriendForm::lastUsername
 * @brief Cached username so we can retranslate the invite message
 */
AddFriendForm::AddFriendForm(QWidget* parent) : QWidget(parent), addUi{new Ui::AddFriendForm} {
    setAttribute(Qt::WA_StyledBackground);
    setLayout(new QGridLayout);
    layout()->setMargin(0);
    layout()->setSpacing(0);

    tabWidget = new QTabWidget(this);
    layout()->addWidget(tabWidget);

    main = new QWidget(tabWidget);
    addUi->setupUi(main);

    connect(addUi->search, &QPushButton::clicked, this, &AddFriendForm::onSearchTriggered);

    connect(this, &AddFriendForm::friendReceipts, this, &AddFriendForm::onFriendReceipts);

    friendLayout = new QVBoxLayout(main);
    friendLayout->setAlignment(Qt::AlignTop);

    addUi->scrollArea->widget()->setLayout(friendLayout);

    tabWidget->addTab(main, "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");

    // accessibility stuff
    addUi->input->setPlaceholderText(tr("Account/E-Mail/Phone Number"));

    retranslateUi();
    settings::Translator::registerHandler(std::bind(&AddFriendForm::retranslateUi, this), this);

    auto _session = ok::Application::Instance()->getSession();
    signIn = &_session->getSignInInfo();
    userService = std::make_unique<ok::backend::UserService>(signIn->stackUrl);
}

AddFriendForm::~AddFriendForm() {
    settings::Translator::unregister(this);
    delete addUi;
}

bool AddFriendForm::isShown() const { return true; }

void AddFriendForm::showTo(ContentLayout* contentLayout) {
    //  contentLayout->mainContent->layout()->addWidget(tabWidget);
    //  contentLayout->mainHead->layout()->addWidget(head);
    //  tabWidget->show();

    //  setIdFromClipboard();
    //  toxId.setFocus();

    // Fix #3421
    // Needed to update tab after opening window
    const int index = tabWidget->currentIndex();
    onCurrentChanged(index);

    int idx = contentLayout->addWidget(this);
    contentLayout->setCurrentIndex(idx);
}

QString AddFriendForm::getMessage() {
    //  message.setPlaceholderText(tr("%1 here! OkEDU me
    //  maybe?").arg(Core::getInstance()->getNick())); const QString msg = message.toPlainText();
    //  return !msg.isEmpty() ? msg : message.placeholderText();
    return tr("hello,I'm ") + Core::getInstance()->getNick();
}

void AddFriendForm::onUsernameSet(const QString& username) { lastUsername = username; }

void AddFriendForm::showEvent(QShowEvent* e) {
    //    onSearchTriggered();
}

void AddFriendForm::onFriendReceipts(const QList<ok::backend::OrgStaff*>& qList) {
    //    auto friendLayout = addUi->scrollAreaWidgetContents;
    qDebug() << "Exist friends:" << friendLayout->count();

    while (friendLayout->count() > 0) {
        auto w = friendLayout->itemAt(0)->widget();
        friendLayout->removeWidget(w);
        disconnect(w);
        delete w;
    }

    for (auto item : qList) {
        if (item->username.isEmpty()) {
            qWarning() << item->name << " have not username!";
            continue;
        }

        item->host = signIn->host;
        auto form = new FriendForm(*item);
        friendLayout->addWidget(form);
        connect(form, &FriendForm::add, [&](const QString& username, QString& nick) {
            qDebug() << "Send request to" << username;
            addFriend(username, nick);
        });
    }
}

void AddFriendForm::searchFriend(const QString& idText) {
    qDebug() << "searchFriend" << idText;
    // 查询账号信息
    userService->search(idText, [this](const QList<ok::backend::OrgStaff*>& qList) {
        emit friendReceipts(qList);
    });
}

void AddFriendForm::addFriend(const QString& idText, const QString& nick) {
    qDebug() << "addFriend" << idText << nick;
    FriendId friendId(idText);
    if (!friendId.isValid()) {
        GUI::showWarning(tr("Couldn't add friend"),
                         tr("%1 Ok ID is invalid", "Tox address error").arg(idText));
        return;
    }

    if (friendId == Core::getInstance()->getSelfId()) {
        // When trying to add your own Ok ID as friend
        GUI::showWarning(tr("Couldn't add friend"), tr("You can't add yourself as a friend!"));
        return;
    }

    auto msg = getMessage();
    emit friendRequested(friendId, nick, msg);
}

void AddFriendForm::onSearchTriggered() {
    const QString id = addUi->input->text();
    searchFriend(id);

    //  this->toxId.clear();
    //  this->message.clear();
}

void AddFriendForm::setIdFromClipboard() {
    const QClipboard* clipboard = QApplication::clipboard();
    const QString trimmedId = clipboard->text().trimmed();
    const QString strippedId = getToxId(trimmedId);
    const Core* core = Core::getInstance();
    const bool isSelf = ToxId::isToxId(strippedId) && ToxId(strippedId) != core->getSelfPeerId();
    if (!strippedId.isEmpty() && ToxId::isToxId(strippedId) && isSelf) {
        //    toxId.setText(trimmedId);
    }
}

void AddFriendForm::onCurrentChanged(int index) {}

void AddFriendForm::retranslateUi() {
    searchButton.setText(tr("Search users"));
    tabWidget->setTabText(0, tr("Add a friend"));
}
