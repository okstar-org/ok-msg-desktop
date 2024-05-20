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
#include "friendform.h"
#include "lib/backend/UserService.h"
#include "lib/settings/translator.h"
#include "src/core/core.h"
#include "src/nexus.h"
#include "src/persistence/settings.h"
#include "src/widget/contentlayout.h"
#include "src/widget/gui.h"
#include "src/widget/style.h"
#include "src/widget/tool/croppinglabel.h"
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
#include "lib/session/AuthSession.h"
#include "ui_addfriendform.h"

namespace {

QString getToxId(const QString &id) {
  return id.trimmed();
}

bool checkIsValidId(const QString &id) { return ToxId::isToxId(id); }

} // namespace

/**
 * @var QString AddFriendForm::lastUsername
 * @brief Cached username so we can retranslate the invite message
 */
AddFriendForm::AddFriendForm(QWidget *parent) : QWidget(parent) ,
    addUi{ new Ui::AddFriendForm}
{

    setLayout(new QGridLayout);

  auto signIn = ok::session::AuthSession::Instance()->getSignInInfo();
  userService = std::make_unique<ok::backend::UserService>(signIn.stackUrl);

  tabWidget = new QTabWidget(this);



  layout()->addWidget(tabWidget);
  layout()->setMargin(0);
  layout()->setSpacing(0);


  main = new QWidget(tabWidget);
//  QFont bold;
//  bold.setBold(true);
//  headLabel.setFont(bold);
//  toxIdLabel.setTextFormat(Qt::RichText);

    addUi->setupUi(main);

//  main->setLayout(&layout);

//  layout.addWidget(&toxIdLabel);
//  layout.addWidget(&toxId);
//  layout.addWidget(&searchButton);

//  friendArea.setWidgetResizable(true);

//  auto scrollAreaWidgetContents = new QWidget();
//  friendArea.setWidget(scrollAreaWidgetContents);

  connect(addUi->search, &QPushButton::clicked, this,
          &AddFriendForm::onSearchTriggered);

  connect(this, &AddFriendForm::friendReceipts, this,
          &AddFriendForm::onFriendReceipts);


  friendLayout= new QVBoxLayout(main);
  friendLayout->setAlignment(Qt::AlignTop);
  addUi->scrollArea->widget()->setLayout(friendLayout);

//  friendArea.widget()->setLayout(&friendLayout);
//  layout.addWidget(&friendArea);

//  layout.addWidget(&messageLabel);
//  layout.addWidget(&message);
//  layout.addWidget(&sendButton);
  tabWidget->addTab(main,{});

  //  importContacts = new QWidget(tabWidget);
  //  importContacts->setLayout(&importContactsLayout);
  //  //importFileLine.addWidget(&importFileLabel);
  //  importFileLine.addStretch();
  //  importFileLine.addWidget(&importFileButton);
  //  importContactsLayout.addLayout(&importFileLine);
  //  importContactsLayout.addWidget(&importMessageLabel);
  //  importContactsLayout.addWidget(&importMessage);
  //  importContactsLayout.addWidget(&importSendButton);
  //  tabWidget->addTab(importContacts, QString());

  QScrollArea *scrollArea = new QScrollArea(tabWidget);
  QWidget *requestWidget = new QWidget(tabWidget);
  scrollArea->setWidget(requestWidget);
  scrollArea->setWidgetResizable(true);

  requestsLayout = new QVBoxLayout(requestWidget);
  requestsLayout->addStretch(1);

  tabWidget->addTab(scrollArea, QString());


//  connect(&toxId, &QLineEdit::returnPressed, this,
//          &AddFriendForm::onSendTriggered);

//  connect(&toxId, &QLineEdit::textChanged, this, &AddFriendForm::onIdChanged);
//  connect(tabWidget, &QTabWidget::currentChanged, this,
//          &AddFriendForm::onCurrentChanged);

//  connect(&sendButton, &QPushButton::clicked, this,
//          &AddFriendForm::onSendTriggered);
//  connect(&importSendButton, &QPushButton::clicked, this,
//          &AddFriendForm::onImportSendClicked);
//  connect(&importFileButton, &QPushButton::clicked, this,
//          &AddFriendForm::onImportOpenClicked);
//  connect(Nexus::getCore(), &Core::usernameSet, this,
//          &AddFriendForm::onUsernameSet);

  // accessibility stuff
  addUi->input->setPlaceholderText(tr("Account/E-Mail/Phone Number"));
//  toxId.setAccessibleDescription(tr("Type in Tox ID of your friend"));
//  messageLabel.setAccessibleDescription(tr("Friend request message"));
//  message.setAccessibleDescription(
//      tr("Type message to send with the friend request or leave empty to send "
//         "a default message"));
//  message.setTabChangesFocus(true);

  retranslateUi();
  settings::Translator::registerHandler(
      std::bind(&AddFriendForm::retranslateUi, this), this);

  const int size = Settings::getInstance().getFriendRequestSize();
  for (int i = 0; i < size; ++i) {
    Settings::Request request = Settings::getInstance().getFriendRequest(i);
    addFriendRequestWidget(request.address, request.message);
  }
}

AddFriendForm::~AddFriendForm() {
  settings::Translator::unregister(this);
    delete addUi;
}

bool AddFriendForm::isShown() const {
  return true;
}

void AddFriendForm::showTo(ContentLayout *contentLayout) {
//  contentLayout->mainContent->layout()->addWidget(tabWidget);
//  contentLayout->mainHead->layout()->addWidget(head);
//  tabWidget->show();

  //  setIdFromClipboard();
//  toxId.setFocus();

  // Fix #3421
  // Needed to update tab after opening window
  const int index = tabWidget->currentIndex();
  onCurrentChanged(index);

  int idx= contentLayout->addWidget(this);
  contentLayout->setCurrentIndex(idx);

}

QString AddFriendForm::getMessage() {
//  message.setPlaceholderText(tr("%1 here! OkEDU me maybe?").arg(Core::getInstance()->getNick()));
//  const QString msg = message.toPlainText();
//  return !msg.isEmpty() ? msg : message.placeholderText();
    return "您好，我是"+Core::getInstance()->getNick();
}

QString AddFriendForm::getImportMessage() const {
  const QString msg = importMessage.toPlainText();
  return msg.isEmpty() ? importMessage.placeholderText() : msg;
}

void AddFriendForm::setMode(Mode mode) { tabWidget->setCurrentIndex(mode); }

bool AddFriendForm::addFriendRequest(const QString &friendAddress,
                                     const QString &message) {
  if (Settings::getInstance().addFriendRequest(friendAddress, message)) {
    addFriendRequestWidget(friendAddress, message);
    if (isShown()) {
      onCurrentChanged(tabWidget->currentIndex());
    }
    return true;
  }
  return false;
}

void AddFriendForm::onUsernameSet(const QString &username) {
    lastUsername = username;
}

void AddFriendForm::showEvent(QShowEvent *e)
{
    onSearchTriggered();
}

void AddFriendForm::onFriendReceipts(const QList<ok::backend::OrgStaff *> &qList) {
//    auto friendLayout = addUi->scrollAreaWidgetContents;
  qDebug() << "Exist friends:" << friendLayout->count();

  while (friendLayout->count() > 0) {
    auto w = friendLayout->itemAt(0)->widget();
    friendLayout->removeWidget(w);
    disconnect(w);
    delete w;
  }

  for (auto item : qList) {
    if (!item->username.isEmpty()) {
      auto form = new FriendForm(item);
      friendLayout->addWidget(form);
      connect(form, &FriendForm::add, [&](QString &username, QString &nick) {
        qDebug() << "Send request to" << username;
        addFriend(username, nick);
      });
    }
  }
}

void AddFriendForm::searchFriend(const QString &idText) {
  qDebug() << "searchFriend" << idText;
  // 查询账号信息
  userService->search(idText, [this](const QList<ok::backend::OrgStaff *> &qList) {
    emit friendReceipts(qList);
  });
}

void AddFriendForm::addFriend(const QString &idText, const QString& nick) {
  qDebug() << "addFriend" << idText<<nick;
  ToxId friendId(idText);
  if (!friendId.isValid()) {
    GUI::showWarning(
        tr("Couldn't add friend"),
        tr("%1 Ok ID is invalid", "Tox address error").arg(idText));
    return;
  }

  deleteFriendRequest(friendId);

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

void AddFriendForm::onSendTriggered() {
  const QString id = addUi->input->text();
  addFriend(id, Nexus::getCore()->getNick());
//  this->toxId.clear();
//  this->message.clear();
}

void AddFriendForm::onImportSendClicked() {
  for (const QString &id : contactsToImport) {
    addFriend(id, Nexus::getCore()->getNick());
  }

  contactsToImport.clear();
  importMessage.clear();
  retranslateUi(); // Update the importFileLabel
}

void AddFriendForm::onImportOpenClicked() {
  const QString path =
      QFileDialog::getOpenFileName(Q_NULLPTR, tr("Open contact list"));
  if (path.isEmpty()) {
    return;
  }

  QFile contactFile(path);
  if (!contactFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
    GUI::showWarning(
        tr("Couldn't open file"),
        //: Error message when trying to open a contact list file to import
        tr("Couldn't open the contact file"));
    return;
  }

  contactsToImport = QString::fromUtf8(contactFile.readAll()).split('\n');
  qDebug() << "Import list:";
  for (auto it = contactsToImport.begin(); it != contactsToImport.end();) {
    const QString id = getToxId(*it);
    if (checkIsValidId(id)) {
      *it = id;
      qDebug() << *it;
      ++it;
    } else {
      if (!id.isEmpty()) {
        qDebug() << "Invalid ID:" << *it;
      }
      it = contactsToImport.erase(it);
    }
  }

  if (contactsToImport.isEmpty()) {
    GUI::showWarning(
        tr("Invalid file"),
        tr("We couldn't find any contacts to import in this file!"));
  }

  retranslateUi(); // Update the importFileLabel to show how many contacts we
                   // have
}

void AddFriendForm::onIdChanged(const QString &id) {
  const QString strippedId = getToxId(id);

  const bool isValidId = checkIsValidId(strippedId);
  //  const bool isValidOrEmpty = strippedId.isEmpty() || isValidId;

  //: Ok ID of the person you're sending a friend request to
  const QString toxIdText(tr("Tox ID"));
  //: Ok ID format description
  const QString toxIdComment(tr("Tox ID comment"));

//  const QString labelText =
      //      isValidId ?
//      QStringLiteral("%1 (%2)");
  //      : QStringLiteral("%1 <font color='red'>(%2)</font>");
//  toxIdLabel.setText(labelText.arg(toxIdText, toxIdComment));

  //  toxId.setStyleSheet(isValidOrEmpty
  //                          ? QStringLiteral("")
  //                          :
  //                          Style::getStylesheet("addFriendForm/toxId.css"));
  //  toxId.setToolTip(isValidOrEmpty ? QStringLiteral("")
  //                                  : tr("Invalid Ok ID format"));

  sendButton.setEnabled(isValidId);
}

void AddFriendForm::setIdFromClipboard() {
  const QClipboard *clipboard = QApplication::clipboard();
  const QString trimmedId = clipboard->text().trimmed();
  const QString strippedId = getToxId(trimmedId);
  const Core *core = Core::getInstance();
  const bool isSelf =
      ToxId::isToxId(strippedId) && ToxId(strippedId) != core->getSelfId();
  if (!strippedId.isEmpty() && ToxId::isToxId(strippedId) && isSelf) {
//    toxId.setText(trimmedId);
  }
}

void AddFriendForm::deleteFriendRequest(const ToxId &toxId) {
  const int size = Settings::getInstance().getFriendRequestSize();
  for (int i = 0; i < size; ++i) {
    Settings::Request request = Settings::getInstance().getFriendRequest(i);
    if (toxId == ToxId(request.address)) {
      Settings::getInstance().removeFriendRequest(i);
      return;
    }
  }
}

void AddFriendForm::onFriendRequestAccepted() {
  QPushButton *acceptButton = static_cast<QPushButton *>(sender());
  QWidget *friendWidget = acceptButton->parentWidget();

  const int index = requestsLayout->indexOf(friendWidget);
  const int indexFromEnd = requestsLayout->count() - index - 2;

  removeFriendRequestWidget(friendWidget);

  const Settings::Request request =
      Settings::getInstance().getFriendRequest(indexFromEnd);
  emit friendRequestAccepted(ToxId(request.address).getPublicKey());
  Settings::getInstance().removeFriendRequest(indexFromEnd);
  Settings::getInstance().savePersonal();
}

void AddFriendForm::onFriendRequestRejected() {
  QPushButton *rejectButton = static_cast<QPushButton *>(sender());
  QWidget *friendWidget = rejectButton->parentWidget();
  const int count= requestsLayout->count();
  const int index = requestsLayout->indexOf(friendWidget);
  const int indexFromEnd = count - index - 2;

  const Settings::Request request =
      Settings::getInstance().getFriendRequest(indexFromEnd);

  emit friendRequestRejected(ToxId(request.address).getPublicKey());

  removeFriendRequestWidget(friendWidget);
  Settings::getInstance().removeFriendRequest(indexFromEnd);
  Settings::getInstance().savePersonal();
}

void AddFriendForm::onCurrentChanged(int index) {
  if (index == FriendRequest &&
      Settings::getInstance().getUnreadFriendRequests() != 0) {
    Settings::getInstance().clearUnreadFriendRequests();
    Settings::getInstance().savePersonal();
    emit friendRequestsSeen();
  }
}

void AddFriendForm::retranslateUi() {
//  headLabel.setText(tr("Add Friends"));

  searchButton.setText(tr("Search users"));
  //: The message you send in friend requests
  static const QString messageLabelText = tr("Message");
  messageLabel.setText(messageLabelText);

  importMessageLabel.setText(messageLabelText);
  //: Button to choose a file with a list of contacts to import
  importFileButton.setText(tr("Open"));
  importSendButton.setText(tr("Send friend requests"));
  sendButton.setText(tr("Send friend request"));

  //: Default message in friend requests if the field is left blank. Write
  //: something appropriate!
//  message.setPlaceholderText(tr("%1 here! OkEDU me maybe?").arg(Core::getInstance()->getUsername()));
//  importMessage.setPlaceholderText(message.placeholderText());

  // importFileLabel.setText(
  //     contactsToImport.isEmpty()
  //         ? tr("Import a list of contacts, one OkID ID per line")
  //         //: Shows the number of contacts we're about to import from a file
  //         (at
  //         //: least one)
  //         : tr("Ready to import %n contact(s), click send to confirm", "",
  //              contactsToImport.size()));

//  onIdChanged(toxId.text());

  tabWidget->setTabText(AddFriend, tr("Add a friend"));
  //  tabWidget->setTabText(ImportContacts, tr("Import contacts"));
  tabWidget->setTabText(FriendRequest, tr("Friend requests"));

  for (QPushButton *acceptButton : acceptButtons) {
    retranslateAcceptButton(acceptButton);
  }

  for (QPushButton *rejectButton : rejectButtons) {
    retranslateRejectButton(rejectButton);
  }
}

void AddFriendForm::addFriendRequestWidget(const QString &friendAddress,
                                           const QString &message) {
  QWidget *friendWidget = new QWidget(tabWidget);
  QHBoxLayout *friendLayout = new QHBoxLayout(friendWidget);
  QVBoxLayout *horLayout = new QVBoxLayout();
  horLayout->setMargin(0);
  friendLayout->addLayout(horLayout);

  CroppingLabel *friendLabel = new CroppingLabel(friendWidget);
  friendLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
  friendLabel->setText("<b>" + friendAddress + "</b>");
  horLayout->addWidget(friendLabel);

  QLabel *messageLabel = new QLabel(message);
  // allow to select text, but treat links as plaintext to prevent phishing
  messageLabel->setTextInteractionFlags(Qt::TextSelectableByMouse |
                                        Qt::TextSelectableByKeyboard);
  messageLabel->setTextFormat(Qt::PlainText);
  messageLabel->setWordWrap(true);
  horLayout->addWidget(messageLabel, 1);

  QPushButton *acceptButton = new QPushButton(friendWidget);
  acceptButtons.append(acceptButton);
  connect(acceptButton, &QPushButton::released, this,
          &AddFriendForm::onFriendRequestAccepted);
  friendLayout->addWidget(acceptButton);
  retranslateAcceptButton(acceptButton);

  QPushButton *rejectButton = new QPushButton(friendWidget);
  rejectButtons.append(rejectButton);
  connect(rejectButton, &QPushButton::released, this,
          &AddFriendForm::onFriendRequestRejected);
  friendLayout->addWidget(rejectButton);
  retranslateRejectButton(rejectButton);

  requestsLayout->insertWidget(0, friendWidget);
}

void AddFriendForm::removeFriendRequestWidget(QWidget *friendWidget) {
  int index = requestsLayout->indexOf(friendWidget);
  requestsLayout->removeWidget(friendWidget);
  acceptButtons.removeAt(index);
  rejectButtons.removeAt(index);
  friendWidget->deleteLater();
}

void AddFriendForm::retranslateAcceptButton(QPushButton *acceptButton) {
  acceptButton->setText(tr("Accept"));
}

void AddFriendForm::retranslateRejectButton(QPushButton *rejectButton) {
  rejectButton->setText(tr("Reject"));
}
