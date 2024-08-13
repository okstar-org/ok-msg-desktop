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

#pragma once

#include "UI/widget/OWidget.h"
#include "lib/session/AuthSession.h"

class QShortcut;
class QPaintEvent;

namespace ok {
class SettingManager;
}

namespace Ui {
class LoginWidget;
}

namespace ok::backend {
class OkCloudService;
}
namespace session {
class AuthSession;
}

namespace UI {

class LoginWidget : public QWidget {
    Q_OBJECT
public:
    explicit LoginWidget(std::shared_ptr<ok::session::AuthSession> session, bool bootstrap,
                         QWidget* parent = nullptr);
    ~LoginWidget() override;
    void onError(int code, const QString& msg);
    void setMsg(const QString& msg);
    void init();
    void deinit();

protected:
    void retranslateUi();
    virtual bool eventFilter(QObject* obj, QEvent* event) override;
    virtual void showEvent(QShowEvent* e) override;

private:
    Ui::LoginWidget* ui;
    std::shared_ptr<ok::session::AuthSession> session;
    bool bootstrap;

    QShortcut* m_loginKey;

    ok::SettingManager* m_settingManager;
    ok::backend::OkCloudService* okCloudService;

    bool m_error = false;

    // 加载项
    int m_loaded;

    QStringList m_hosts;
    QStringList m_stacks;
    std::unique_ptr<QTimer> m_timer;
    QString m_currentOriginalMsg{""};

signals:
    void loginSuccess(QString name, QString password);
    void loginFailed(QString name, QString password);
    void loginTimeout(QString name);

private slots:
    void onTimeout();
    void doLogin();
    void onLoginResult(ok::session::SignInInfo info, ok::session::LoginResult result);
    void on_loginBtn_released();
    void on_language_currentIndexChanged(int index);
    void on_providers_currentIndexChanged(int index);
};

}  // namespace UI
