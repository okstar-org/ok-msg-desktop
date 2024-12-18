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

#include "chatform.h"
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QFileInfo>
#include "lib/settings/translator.h"
#include "src/base/MessageBox.h"
#include "src/chatlog/chatlinecontentproxy.h"
#include "src/chatlog/chatlog.h"
#include "src/chatlog/chatmessage.h"
#include "src/chatlog/content/filetransferwidget.h"
#include "src/chatlog/content/text.h"
#include "src/core/core.h"
#include "src/core/coreav.h"
#include "src/core/corefile.h"
#include "src/lib/settings/style.h"
#include "src/model/friend.h"
#include "src/model/status.h"
#include "src/nexus.h"
#include "src/persistence/history.h"
#include "src/persistence/offlinemsgengine.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/video/netcamview.h"
#include "src/widget/chatformheader.h"
#include "src/widget/form/loadhistorydialog.h"
#include "src/widget/maskablepixmapwidget.h"
#include "src/widget/tool/callconfirmwidget.h"
#include "src/widget/tool/chattextedit.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/tool/screenshotgrabber.h"
#include "src/widget/widget.h"

#include <QMimeData>
#include <QPushButton>
#include <QScrollBar>
#include <QSplitter>
#include <QStringBuilder>

#include <cassert>

/**
 * @brief ChatForm::incomingNotification Notify that we are called by someone.
 * @param friendId IMFriend that is calling us.
 *
 * @brief ChatForm::outgoingNotification Notify that we are calling someone.
 *
 * @brief stopNotification Tell others to stop notification of a call.
 */

static constexpr int CHAT_WIDGET_MIN_HEIGHT = 50;
static constexpr int SCREENSHOT_GRABBER_OPENING_DELAY = 500;

const QString ChatForm::ACTION_PREFIX = QStringLiteral("/me ");

ChatForm::ChatForm(const FriendId* chatFriend,
                   IChatLog& chatLog_,
                   IMessageDispatcher& messageDispatcher)
        : GenericChatForm(chatFriend, chatLog_, messageDispatcher), f(chatFriend) {
    //      headWidget->setAvatar(QPixmap(":/img/contact_dark.svg"));

    statusMessageLabel = new CroppingLabel();
    statusMessageLabel->setObjectName("statusLabel");
    statusMessageLabel->setFont(Style::getFont(Style::Medium));
    statusMessageLabel->setMinimumHeight(Style::getFont(Style::Medium).pixelSize());
    statusMessageLabel->setTextFormat(Qt::PlainText);
    statusMessageLabel->setContextMenuPolicy(Qt::CustomContextMenu);

    chatLog->setTypingNotification(ChatMessage::createTypingNotification());
    chatLog->setMinimumHeight(CHAT_WIDGET_MIN_HEIGHT);

    //  headWidget->addWidget(statusMessageLabel);
    //  headWidget->addStretch();
    //  headWidget->addWidget(callDuration, 1, Qt::AlignCenter);

    copyStatusAction = statusMessageMenu.addAction(QString(), this, SLOT(onCopyStatusMessage()));

    //  const Core *core = Core::getInstance();
    //  const Profile *profile = Nexus::getProfile();
    //  const CoreFile *coreFile = core->getCoreFile();
    //  connect(profile, &Profile::friendAvatarChanged, this,
    //          &ChatForm::onAvatarChanged);
    //  connect(coreFile, &CoreFile::fileReceiveRequested, this,
    //          &ChatForm::updateFriendActivityForFile);
    //  connect(coreFile, &CoreFile::fileSendStarted, this,
    //          &ChatForm::updateFriendActivityForFile);

    //  connect(coreFile, &CoreFile::fileNameChanged, this,
    //          &ChatForm::onFileNameChanged);

    //  connect(headWidget, &ChatFormHeader::micMuteToggle, this,
    //          &ChatForm::onMicMuteToggle);
    //  connect(headWidget, &ChatFormHeader::volMuteToggle, this,
    //          &ChatForm::onVolMuteToggle);

    //    connect(sendButton, &QPushButton::pressed, this, &ChatForm::callUpdateFriendActivity);
    //    connect(msgEdit, &ChatTextEdit::enterPressed, this, &ChatForm::callUpdateFriendActivity);

    //    connect(msgEdit, &ChatTextEdit::pasteImage, this, &ChatForm::sendImage);
    connect(statusMessageLabel, &CroppingLabel::customContextMenuRequested, this,
            [&](const QPoint& pos) {
                if (!statusMessageLabel->text().isEmpty()) {
                    QWidget* sender = static_cast<QWidget*>(this->sender());
                    statusMessageMenu.exec(sender->mapToGlobal(pos));
                }
            });

    connect(&typingTimer, &QTimer::timeout, this, [this] {
        Core::getInstance()->sendTyping(f->toString(), false);
        isTyping = false;
    });

    setAcceptDrops(true);

    settings::Translator::registerHandler(std::bind(&ChatForm::retranslateUi, this), this);
    retranslateUi();
}

ChatForm::~ChatForm() { settings::Translator::unregister(this); }

void ChatForm::setStatusMessage(const QString& newMessage) {
    statusMessageLabel->setText(newMessage);
    // for long messsages
    statusMessageLabel->setToolTip(Qt::convertFromPlainText(newMessage, Qt::WhiteSpaceNormal));
}

void ChatForm::callUpdateFriendActivity() { emit updateFriendActivity(*f); }

void ChatForm::updateFriendActivityForFile(const ToxFile& file) {
    if (file.receiver != f->getId()) {
        return;
    }
    emit updateFriendActivity(*f);
}

void ChatForm::onFileNameChanged(const FriendId& friendPk) {
    if (friendPk != *f) {
        return;
    }

    ok::base::MessageBox::warning(this, tr("Filename contained illegal characters"),
                                  tr("Illegal characters have been changed to _ \n"
                            "so you can save the file on windows."));
}

void ChatForm::showOutgoingCall(bool video) {
    //  headWidget->showOutgoingCall(video);
    addSystemInfoMessage(tr("Calling %1").arg(f->username), ChatMessage::INFO,
                         QDateTime::currentDateTime());
    emit outgoingNotification();
    emit updateFriendActivity(*f);
}

void ChatForm::onFriendStatusChanged(const FriendId& friendId, Status::Status status) {
    qDebug() << __func__ << friendId.toString() << (int)status;
    // Disable call buttons if friend is offline
    if (friendId.toString() != f->getId()) {
        return;
    }

    //  if (!Status::isOnline(f->getStatus())) {
    // Hide the "is typing" message when a friend goes offline
    //    setFriendTyping(false);
    //  }

    //  updateCallButtons();

    //  if (Settings::getInstance().getStatusChangeNotificationEnabled()) {
    //    QString fStatus = Status::getSubject(status);
    //    addSystemInfoMessage(tr("%1 is now %2", "e.g. \"Dubslow is now online\"")
    //                             .arg(f->getDisplayedName())
    //                             .arg(fStatus),
    //                         ChatMessage::INFO, QDateTime::currentDateTime());
    //  }
}

void ChatForm::onFriendNameChanged(const QString& name) {
    qDebug() << __func__ << name;
    //  if (sender() == f->toString()) {
    //    setName(name);
    //  }
}

void ChatForm::onStatusMessage(const QString& message) {
    qDebug() << __func__ << message;
    //  if (sender() == f) {
    //    setStatusMessage(message);
    //  }
}

void ChatForm::dragEnterEvent(QDragEnterEvent* ev) {
    if (ev->mimeData()->hasUrls()) {
        ev->acceptProposedAction();
    }
}

void ChatForm::dropEvent(QDropEvent* ev) {
    if (!ev->mimeData()->hasUrls()) {
        return;
    }

    Core* core = Core::getInstance();
    for (const QUrl& url : ev->mimeData()->urls()) {
        QFileInfo info(url.path());
        QFile file(info.absoluteFilePath());

        QString urlString = url.toString();
        if (url.isValid() && !url.isLocalFile() && urlString.length() < 1024) {
            messageDispatcher.sendMessage(false, urlString);

            continue;
        }

        QString fileName = info.fileName();
        if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
            info.setFile(url.toLocalFile());
            file.setFileName(info.absoluteFilePath());
            if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
                ok::base::MessageBox::warning(this, tr("Unable to open"),
                                              tr("OkMsg wasn't able to open %1").arg(fileName));
                continue;
            }
        }

        file.close();
        if (file.isSequential()) {
            ok::base::MessageBox::critical(nullptr, tr("Bad idea"),
                                           tr("You're trying to send a sequential file, "
                                     "which is not going to work!"));
            continue;
        }

        if (info.exists()) {
            CoreFile::getInstance()->sendFile(f->getId(), file);
        }
    }
}

void ChatForm::clearChatArea() {
    GenericChatForm::clearChatArea(/* confirm = */ false, /* inform = */ true);
}



void ChatForm::sendImage(const QPixmap& pixmap) {
    QDir(Settings::getInstance().getAppDataDirPath()).mkpath("images");

    // use ~ISO 8601 for screenshot timestamp, considering FS limitations
    // https://en.wikipedia.org/wiki/ISO_8601
    // Windows has to be supported, thus filename can't have `:` in it :/
    // Format should be: `qTox_Screenshot_yyyy-MM-dd HH-mm-ss.zzz.png`
    QString filepath =
            QString("%1images%2qTox_Image_%3.png")
                    .arg(Settings::getInstance().getAppDataDirPath())
                    .arg(QDir::separator())
                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH-mm-ss.zzz"));
    QFile file(filepath);

    if (file.open(QFile::ReadWrite)) {
        pixmap.save(&file, "PNG");
        qint64 filesize = file.size();
        file.close();

        CoreFile* coreFile = CoreFile::getInstance();
        coreFile->sendFile(f->getId(), file);
    } else {
        ok::base::MessageBox::warning(
                this,
                tr("Failed to open temporary file", "Temporary file for screenshot"),
                tr("OkMsg wasn't able to save the screenshot"));
    }
}

void ChatForm::insertChatMessage(IChatItem::Ptr msg) { GenericChatForm::insertChatMessage(msg); }

void ChatForm::onCopyStatusMessage() {
    qDebug() << __func__;
    //  QString text = f->getStatusMessage();
    //  QClipboard *clipboard = QApplication::clipboard();
    //  if (clipboard) {
    //    clipboard->setText(text, QClipboard::Clipboard);
    //  }
}

void ChatForm::setFriendTyping(bool typing) {
    isTyping = typing;
    if (chatLog) chatLog->setTypingNotificationVisible(typing);
    //  QString typingDiv = "<div class=typing>%1</div>";
    //  QString name = f->getDisplayedName();
    //  Text *text = static_cast<Text *>(chatLog->getTypingNotification()->centerContent());
    //  text->setText(typingDiv.arg(tr("%1 is typing").arg(name)));
}

void ChatForm::show(ContentLayout* contentLayout) { GenericChatForm::show(contentLayout); }

void ChatForm::reloadTheme() {
    chatLog->setTypingNotification(ChatMessage::createTypingNotification());
    GenericChatForm::reloadTheme();
}

void ChatForm::showEvent(QShowEvent* event) {
    //  GenericChatForm::showEvent(event);
}

void ChatForm::hideEvent(QHideEvent* event) {
    //  GenericChatForm::hideEvent(event);
}

void ChatForm::retranslateUi() { copyStatusAction->setText(tr("Copy")); }
