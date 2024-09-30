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

//
// Created by gaojie on 24-9-29.
//

#include "ChatInputForm.h"

#include <QDebug>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QVBoxLayout>

#include "lib/settings/style.h"
#include "src/persistence/profile.h"
#include "src/widget/tool/screenshotgrabber.h"

#ifdef OK_PLUGIN
#include <lib/settings/settings.h>
#include <src/chatlog/chatmessageitem.h>
#include <src/core/core.h>
#include <src/core/coreav.h>
#include <src/nexus.h>
#include <src/widget/tool/chattextedit.h>
#include <QFileDialog>
#include <QMessageBox>
#include "lib/plugin/pluginmanager.h"
#endif

const QString STYLE_PATH = QStringLiteral("chatForm/buttons.css");
static const short FOOT_BUTTONS_SPACING = 2;
static const short MESSAGE_EDIT_HEIGHT = 50;
static const QString FONT_STYLE[]{"normal", "italic", "oblique"};
static constexpr int SCREENSHOT_GRABBER_OPENING_DELAY = 500;

static QString fontToCss(const QFont& font, const QString& name) {
    QString result{
            "%1{"
            "font-family: \"%2\"; "
            "font-size: %3px; "
            "font-style: \"%4\"; "
            "font-weight: normal;}"};
    return result.arg(name).arg(font.family()).arg(font.pixelSize()).arg(FONT_STYLE[font.style()]);
}

template <class T, class Fun>
QPushButton* createButton(const QString& name, T* self, Fun onClickSlot) {
    QPushButton* btn = new QPushButton();
    // Fix for incorrect layouts on OS X as per
    // https://bugreports.qt-project.org/browse/QTBUG-14591
    btn->setAttribute(Qt::WA_LayoutUsesWidgetRect);
    btn->setCursor(Qt::PointingHandCursor);
    btn->setObjectName(name);
    // btn->setProperty("state", "green");
    btn->setStyleSheet(Style::getStylesheet(STYLE_PATH));
    btn->setCheckable(true);
    QObject::connect(btn, &QPushButton::clicked, self, onClickSlot);
    return btn;
}

ChatInputForm::ChatInputForm(QWidget* parent, bool isGroup) : QWidget(parent), isGroup{isGroup} {
    setContentsMargins(0, 0, 0, 0);
    setObjectName("ChatInputForm");

    if (isGroup) {
        fileButton->setEnabled(false);
        fileButton->setProperty("state", "disabled");
    }

#ifdef SPELL_CHECKING
    if (s.getSpellCheckingEnabled()) {
        decorator = new Sonnet::SpellCheckDecorator(msgEdit);
    }
#endif
    encryptButton = createButton("encryptButton", this, &ChatInputForm::onEncryptButtonClicked);

    emoteButton = createButton("emoteButton", this, &ChatInputForm::onEmoteButtonClicked);
    fileButton = createButton("fileButton", this, &ChatInputForm::onAttachClicked);
    screenshotButton = createButton("screenshotButton", this, &ChatInputForm::onScreenshotClicked);
    sendButton = createButton("sendButton", this, &ChatInputForm::onSendTriggered);

    // connect(bodySplitter, &QSplitter::splitterMoved, this, &ChatInputForm::onSplitterMoved);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(FOOT_BUTTONS_SPACING);

    QHBoxLayout* ctrlLayout = new QHBoxLayout();
    ctrlLayout->setSpacing(FOOT_BUTTONS_SPACING);
    ctrlLayout->addWidget(emoteButton);
    ctrlLayout->addWidget(fileButton);
    ctrlLayout->addWidget(screenshotButton);
    ctrlLayout->addSpacerItem(
            new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));
#ifdef OK_PLUGIN
    auto pm = ok::plugin::PluginManager::instance();
    connect(pm, &ok::plugin::PluginManager::pluginEnabled,  //
            this, &::ChatInputForm::onPluginEnabled);
    connect(pm, &ok::plugin::PluginManager::pluginDisabled,  //
            this, &::ChatInputForm::onPluginDisabled);
    auto omemo = pm->plugin("omemo");
    if (omemo) {
        ctrlLayout->addWidget(encryptButton);
    }
#endif
    ctrlLayout->addStretch(1);
    ctrlLayout->addWidget(sendButton);
    mainLayout->addLayout(ctrlLayout);

    auto sendLayout = new QHBoxLayout();

    msgEdit = new ChatTextEdit();
    msgEdit->setFrameStyle(QFrame::NoFrame);

    connect(msgEdit, &ChatTextEdit::textChanged, this, &ChatInputForm::onTextEditChanged);
    connect(msgEdit, &ChatTextEdit::enterPressed, this, &ChatInputForm::onSendTriggered);
    // 添加输入框
    sendLayout->addWidget(msgEdit);

    mainLayout->addLayout(sendLayout);

    setLayout(mainLayout);

    reloadTheme();
    retranslateUi();
}

void ChatInputForm::reloadTheme() {
    const Settings& s = Settings::getInstance();
    msgEdit->setStyleSheet(Style::getStylesheet("msgEdit/msgEdit.css") +
                           fontToCss(s.getChatMessageFont(), "QTextEdit"));
    fileButton->setStyleSheet(Style::getStylesheet(STYLE_PATH));
    emoteButton->setStyleSheet(Style::getStylesheet(STYLE_PATH));
    screenshotButton->setStyleSheet(Style::getStylesheet(STYLE_PATH));
    sendButton->setStyleSheet(Style::getStylesheet(STYLE_PATH));
}

void ChatInputForm::retranslateUi() {
    sendButton->setToolTip(tr("Send message"));
    emoteButton->setToolTip(tr("Smileys"));
    fileButton->setToolTip(tr("Send file(s)"));
    screenshotButton->setToolTip(tr("Send a screenshot"));
}

void ChatInputForm::keyPressEvent(QKeyEvent* ev) {
    if (msgEdit->hasFocus()) return;
}

void ChatInputForm::keyReleaseEvent(QKeyEvent* ev) {
    // onSendTriggered();
}

void ChatInputForm::dragEnterEvent(QDragEnterEvent* ev) {}

void ChatInputForm::dropEvent(QDropEvent* ev) {}

void ChatInputForm::setFocus() { msgEdit->setFocus(); }

void ChatInputForm::updateFont(const QFont& font) {
    // message editor
    msgEdit->setStyleSheet(Style::getStylesheet("msgEdit/msgEdit.css") +
                           fontToCss(font, "QTextEdit"));
}

QString ChatInputForm::getInputText() { return msgEdit->toPlainText(); }

#ifdef OK_PLUGIN
void ChatInputForm::onPluginEnabled(const QString& shortName) {
    //  qDebug() << "Plugin is enabled" << shortName <<"for"<<contact->getDisplayedName();
    if (shortName == "omemo") {
        auto encryptButton_ = mainFootLayout->findChild<QPushButton*>("encryptButton");
        if (!encryptButton_) {
            mainFootLayout->insertWidget(0, encryptButton);
        }
        encryptButton->show();
    }
}

void ChatInputForm::onPluginDisabled(const QString& shortName) {
    qDebug() << "Plugin is disabled." << shortName;
    if (shortName == "omemo") {
        encryptButton->hide();
        //    mainFootLayout->removeWidget(encryptButton);
    }
}
#endif

void ChatInputForm::onSendTriggered() {
    auto msg = msgEdit->toPlainText();
    if (msg.isEmpty()) {
        return;
    }

    msgEdit->setLastMessage(msg);
    msgEdit->clear();

    qDebug() << "Input text:" << msg;
    emit inputText(msg);
}

void ChatInputForm::onAttachClicked() {
    qDebug() << __func__;

    QStringList paths = QFileDialog::getOpenFileNames(Q_NULLPTR, tr("Send a file"),
                                                      QDir::homePath(), nullptr, nullptr);

    if (paths.isEmpty()) {
        return;
    }

    Core* core = Core::getInstance();
    for (QString path : paths) {
        QFile file(path);

        if (!file.exists() || !file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, tr("Unable to open"),
                                 tr("Wasn't able to open %1").arg(path));
            continue;
        }

        file.close();
        if (file.isSequential()) {
            QMessageBox::critical(this, tr("Bad idea"),
                                  tr("You're trying to send a sequential file, "
                                     "which is not going to work!"));
            continue;
        }

        // qint64 filesize = file.size();
        qDebug() << "sending" << file;
        emit inputFile(file);
    }
}

void ChatInputForm::onTextEditChanged() {
    auto text = msgEdit->toPlainText();
    emit inputTextChanged(text);
}

void ChatInputForm::onScreenshotClicked() { doScreenshot(); }

void ChatInputForm::onScreenCaptured(const QPixmap& pixmap) {
    if (pixmap.isNull()) {
        qWarning() << "Empty picture captured!";
        return;
    }
    emit inputScreenCapture(pixmap);
}

void ChatInputForm::doScreenshot() {
    // note: grabber is self-managed and will destroy itself when done
    ScreenshotGrabber* grabber = new ScreenshotGrabber;
    connect(grabber, &ScreenshotGrabber::screenshotTaken, this, &ChatInputForm::onScreenCaptured);
    grabber->showGrabber();
}