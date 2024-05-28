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

#include "src/net/toxuri.h"
#include "src/core/core.h"
#include "src/nexus.h"
#include "src/widget/gui.h"
#include "src/widget/tool/friendrequestdialog.h"
#include <QByteArray>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QString>
#include <QThread>
#include <QVBoxLayout>

bool toxURIEventHandler(const QByteArray& eventData)
{
    if (!eventData.startsWith("tox:"))
        return false;

    handleToxURI(eventData);
    return true;
}

/**
 * @brief Shows a dialog asking whether or not to add this tox address as a friend.
 * @note Will wait until the core is ready first.
 * @param toxURI Tox URI to try to add.
 * @return True, if tox URI is correct, false otherwise.
 */
bool handleToxURI(const QString& toxURI)
{
    Nexus& nexus = Nexus::getInstance();
    Core* core = nexus.getCore();

    while (!core) {
        core = nexus.getCore();
        qApp->processEvents();
        QThread::msleep(10);
    }

    QString toxaddr = toxURI.mid(4);

    ToxId toxId(toxaddr);
    QString error = QString();
    if (!toxId.isValid()) {
        error = QMessageBox::tr("%1 is not a valid Tox address.").arg(toxaddr);
    } else if (toxId == core->getSelfId()) {
        error = QMessageBox::tr("You can't add yourself as a friend!",
                                "When trying to add your own Ok ID as friend");
    }

    if (!error.isEmpty()) {
        GUI::showWarning(QMessageBox::tr("Couldn't add friend"), error);
        return false;
    }

    const QString defaultMessage =
        QObject::tr("%1 here! OkEDU me maybe?",
                    "Default message in OkEDU URI friend requests. Write something appropriate!");
    const QString username = Nexus::getCore()->getUsername();
    ToxURIDialog* dialog = new ToxURIDialog(nullptr, toxaddr, defaultMessage.arg(username));
//    QObject::connect(dialog, &ToxURIDialog::finished, [&](int result) {
//        if (result == QDialog::Accepted) {
//            Core::getInstance()->requestFriendship(toxId,Nexus::getCore()->getNick(), dialog->getRequestMessage());
//        }

//        dialog->deleteLater();
//    });

    dialog->open();

    return true;
}

ToxURIDialog::ToxURIDialog(QWidget* parent, const QString& userId, const QString& message)
    : QDialog(parent)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowTitle(tr("Add a friend", "Title of the window to add a friend through Tox URI"));

    QLabel* friendsLabel = new QLabel(tr("Do you want to add %1 as a friend?").arg(userId), this);
    QLabel* userIdLabel = new QLabel(tr("User ID:"), this);
    QLineEdit* userIdEdit = new QLineEdit(userId, this);
    userIdEdit->setCursorPosition(0);
    userIdEdit->setReadOnly(true);
    QLabel* messageLabel = new QLabel(tr("IMFriend request message:"), this);
    messageEdit = new QPlainTextEdit(message, this);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal, this);

    buttonBox->addButton(tr("Send", "Send a friend request"), QDialogButtonBox::AcceptRole);
    buttonBox->addButton(tr("Cancel", "Don't send a friend request"), QDialogButtonBox::RejectRole);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &FriendRequestDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &FriendRequestDialog::reject);

    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(friendsLabel);
    layout->addSpacing(12);
    layout->addWidget(userIdLabel);
    layout->addWidget(userIdEdit);
    layout->addWidget(messageLabel);
    layout->addWidget(messageEdit);
    layout->addWidget(buttonBox);

    resize(300, 200);
}

QString ToxURIDialog::getRequestMessage()
{
    return messageEdit->toPlainText();
}
