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

#ifndef LOGINSCREEN_H
#define LOGINSCREEN_H

#include <QDialog>
#include <QShortcut>
#include <QToolButton>

class Profile;

namespace Ui {
class LoginScreen;
}

class LoginScreen : public QDialog {
    Q_OBJECT

public:
    LoginScreen(const QString& initialProfileName = QString(), QWidget* parent = nullptr);
    ~LoginScreen();
    bool event(QEvent* event) final override;

signals:

    void windowStateChanged(Qt::WindowStates states);
    void autoLoginChanged(bool state);
    void createNewProfile(QString name, const QString& pass);
    void loadProfile(QString name, const QString& pass);

public slots:
    void onProfileLoaded();
    void onProfileLoadFailed();
    void onAutoLoginChanged(bool state);

private slots:
    void onAutoLoginCheckboxChanged(int state);
    void onLoginUsernameSelected(const QString& name);
    void onPasswordEdited();
    // Buttons to change page
    void onNewProfilePageClicked();
    void onLoginPageClicked();
    // Buttons to submit form
    void onCreateNewProfile();
    void onLogin();
    void onImportProfile();

private:
    void reset(const QString& initialProfileName = QString());
    void retranslateUi();
    void showCapsIndicator();
    void hideCapsIndicator();
    void checkCapsLock();

private:
    Ui::LoginScreen* ui;
    QShortcut quitShortcut;
};

#endif  // LOGINSCREEN_H
