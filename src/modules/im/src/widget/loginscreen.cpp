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

#include "loginscreen.h"
#include <QDebug>
#include <QDialog>
#include <QMessageBox>
#include <QToolButton>
#include "lib/settings/translator.h"
#include "src/lib/settings/style.h"
#include "src/persistence/profile.h"
#include "src/persistence/profilelocker.h"
#include "src/persistence/settings.h"
#include "src/widget/form/setpassworddialog.h"
#include "src/widget/tool/profileimporter.h"
#include "ui_loginscreen.h"

LoginScreen::LoginScreen(const QString& initialProfileName, QWidget* parent)
        : QDialog(parent)
        , ui(new Ui::LoginScreen)
        , quitShortcut{QKeySequence(Qt::CTRL + Qt::Key_Q), this} {
    ui->setupUi(this);

    // permanently disables maximize button https://github.com/qTox/qTox/issues/1973
    this->setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);
    this->setFixedSize(this->size());

    connect(&quitShortcut, &QShortcut::activated, this, &LoginScreen::close);
    connect(ui->newProfilePgbtn, &QPushButton::clicked, this,
            &LoginScreen::onNewProfilePageClicked);
    connect(ui->loginPgbtn, &QPushButton::clicked, this, &LoginScreen::onLoginPageClicked);
    connect(ui->createAccountButton, &QPushButton::clicked, this, &LoginScreen::onCreateNewProfile);
    connect(ui->newUsername, &QLineEdit::returnPressed, this, &LoginScreen::onCreateNewProfile);
    connect(ui->newPass, &QLineEdit::returnPressed, this, &LoginScreen::onCreateNewProfile);
    connect(ui->newPassConfirm, &QLineEdit::returnPressed, this, &LoginScreen::onCreateNewProfile);
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginScreen::onLogin);
    connect(ui->loginUsernames, &QComboBox::currentTextChanged, this,
            &LoginScreen::onLoginUsernameSelected);
    connect(ui->loginPassword, &QLineEdit::returnPressed, this, &LoginScreen::onLogin);
    connect(ui->newPass, &QLineEdit::textChanged, this, &LoginScreen::onPasswordEdited);
    connect(ui->newPassConfirm, &QLineEdit::textChanged, this, &LoginScreen::onPasswordEdited);
    connect(ui->autoLoginCB, &QCheckBox::stateChanged, this,
            &LoginScreen::onAutoLoginCheckboxChanged);
    connect(ui->importButton, &QPushButton::clicked, this, &LoginScreen::onImportProfile);

    reset(initialProfileName);
    this->setStyleSheet(Style::getStylesheet("loginScreen/loginScreen.css"));

    retranslateUi();
    settings::Translator::registerHandler(std::bind(&LoginScreen::retranslateUi, this), this);
}

LoginScreen::~LoginScreen() {
    settings::Translator::unregister(this);
    delete ui;
}

/**
 * @brief Resets the UI, clears all fields.
 */
void LoginScreen::reset(const QString& initialProfileName) {
    ui->newUsername->clear();
    ui->newPass->clear();
    ui->newPassConfirm->clear();
    ui->loginPassword->clear();
    ui->loginUsernames->clear();

    QStringList allProfileNames = Profile::getAllProfileNames();

    if (allProfileNames.isEmpty()) {
        ui->stackedWidget->setCurrentIndex(0);
        ui->newUsername->setFocus();
    } else {
        for (const QString& profileName : allProfileNames) {
            ui->loginUsernames->addItem(profileName);
        }

        ui->loginUsernames->setCurrentText(initialProfileName);
        ui->stackedWidget->setCurrentIndex(1);
        ui->loginPassword->setFocus();
    }
}

void LoginScreen::onProfileLoaded() { done(QDialog::Accepted); }

void LoginScreen::onProfileLoadFailed() {
    QMessageBox::critical(this, tr("Couldn't load this profile"), tr("Wrong password."));
    ui->loginPassword->setFocus();
    ui->loginPassword->selectAll();
}

void LoginScreen::onAutoLoginChanged(bool state) { ui->autoLoginCB->setChecked(state); }

bool LoginScreen::event(QEvent* event) {
    switch (event->type()) {
#ifdef Q_OS_MAC
        case QEvent::WindowActivate:
        case QEvent::WindowStateChange:
            emit windowStateChanged(windowState());
            break;
#endif
        default:
            break;
    }

    return QWidget::event(event);
}

void LoginScreen::onNewProfilePageClicked() { ui->stackedWidget->setCurrentIndex(0); }

void LoginScreen::onLoginPageClicked() { ui->stackedWidget->setCurrentIndex(1); }

void LoginScreen::onCreateNewProfile() {
    QString name = ui->newUsername->text();
    QString pass = ui->newPass->text();

    if (name.isEmpty()) {
        QMessageBox::critical(this, tr("Couldn't create a new profile"),
                              tr("The username must not be empty."));
        return;
    }

    if (pass.size() != 0 && pass.size() < 6) {
        QMessageBox::critical(this, tr("Couldn't create a new profile"),
                              tr("The password must be at least 6 characters long."));
        return;
    }

    if (ui->newPassConfirm->text() != pass) {
        QMessageBox::critical(this, tr("Couldn't create a new profile"),
                              tr("The passwords you've entered are different.\nPlease make sure to "
                                 "enter same password twice."));
        return;
    }

    if (Profile::exists(name)) {
        QMessageBox::critical(this, tr("Couldn't create a new profile"),
                              tr("A profile with this name already exists."));
        return;
    }

    emit createNewProfile(name, pass);
}

void LoginScreen::onLoginUsernameSelected(const QString& name) {
    if (name.isEmpty()) return;

    ui->loginPassword->clear();
    if (Profile::isEncrypted(name)) {
        ui->loginPasswordLabel->show();
        ui->loginPassword->show();
        // there is no way to do autologin if profile is encrypted, and
        // visible option confuses users into thinking that it is possible,
        // thus hide it
        ui->autoLoginCB->hide();
    } else {
        ui->loginPasswordLabel->hide();
        ui->loginPassword->hide();
        ui->autoLoginCB->show();
        ui->autoLoginCB->setToolTip(
                tr("Password protected profiles can't be automatically loaded."));
    }
}

void LoginScreen::onLogin() {
    QString name = ui->loginUsernames->currentText();
    QString pass = ui->loginPassword->text();

    // name can be empty when there are no profiles
    if (name.isEmpty()) {
        QMessageBox::critical(this, tr("Couldn't load profile"),
                              tr("There is no selected profile.\n\n"
                                 "You may want to create one."));
        return;
    }

    if (!ProfileLocker::isLockable(name)) {
        QMessageBox::critical(this, tr("Couldn't load this profile"),
                              tr("This profile is already in use."));
        return;
    }

    emit loadProfile(name, pass);
}

void LoginScreen::onPasswordEdited() {
    ui->passStrengthMeter->setValue(SetPasswordDialog::getPasswordStrength(ui->newPass->text()));
}

void LoginScreen::onAutoLoginCheckboxChanged(int state) {
    auto cstate = static_cast<Qt::CheckState>(state);
    emit autoLoginChanged(cstate == Qt::CheckState::Checked);
}

void LoginScreen::retranslateUi() { ui->retranslateUi(this); }

void LoginScreen::onImportProfile() {
    ProfileImporter pi(this);
    if (pi.importProfile()) {
        reset();
    }
}
