﻿/*
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

#include "LoginWidget.h"

#include <QDesktopServices>
#include <QPaintEvent>
#include <QShortcut>
#include <QTranslator>
#include <QWidget>
#include <memory>

#include "SettingManager.h"
#include "lib/backend/OkCloudService.h"
#include "lib/storage/settings/OkSettings.h"
#include "lib/storage/settings/translator.h"
#include "ui_LoginWidget.h"
#include "application.h"

namespace UI {

using namespace ok;
using namespace lib::session;
using namespace ok::base;

/**
 * 登录控件
 * @param session
 * @param bootstrap
 * @param parent
 */
LoginWidget::LoginWidget(std::shared_ptr<lib::session::AuthSession> session, bool bootstrap,
                         QWidget* parent)
        : QWidget(parent)
        , ui(new Ui::LoginWidget)
        , session(session)
        , bootstrap{bootstrap}
        , m_loginKey(nullptr)
        , m_settingManager(new SettingManager(this))
        , m_loaded(0) {
    qDebug() << __func__;
    ui->setupUi(this);

    // return keyboard
    m_loginKey = new QShortcut(QKeySequence(Qt::Key_Return), this);
    connect(m_loginKey, SIGNAL(activated()), this, SLOT(on_loginBtn_released()));

    // settings
    //  ui->settings->hide();
    ui->settings->setCursor(Qt::PointingHandCursor);
    ui->settings->installEventFilter(this);

    // timer for login #TODO need to refactor
    if (bootstrap) {
        qDebug() << __func__ << "Init timer";
        m_timer = std::make_unique<QTimer>();
        m_timer->start(3000);
        connect(m_timer.get(), &QTimer::timeout, this, &LoginWidget::onTimeout);
    }

    // session
    connect(session.get(),                            //
            &lib::session::AuthSession::loginResult,  //
            this, &LoginWidget::onLoginResult);
    init();

    // translator
    retranslateUi();
    auto a = ok::Application::Instance();
    connect(a->bus(), &ok::Bus::languageChanged,this,
            [&](const QString& locale0) {
                retranslateUi();
            });
}

LoginWidget::~LoginWidget() {
    qDebug() << __func__;
    
    delete ui;
}

void LoginWidget::init() {
    // 1. get remember account
    m_settingManager->getAccount([&](const QString& acc, const QString& password) {
        ui->rember->setChecked(!acc.isEmpty());
        ui->accountInput->setText(acc);
        ui->passwordInput->setText(password);
    });

    // 2. i18n
    auto& setting = lib::settings::OkSettings::getInstance();
    qDebug() << "Last settings translation:" << setting.getTranslation();

    for (int i = 0; i < setting.getLocales().size(); ++i) {
        auto& locale = setting.getLocales().at(i);
        QString langName = QLocale(locale).nativeLanguageName();
        ui->language->addItem(langName);
    }
    // set default
    auto i = setting.getLocales().indexOf(setting.getTranslation());
    if (i >= 0 && i < ui->language->count()) ui->language->setCurrentIndex(i + 1);

    // 3.provider
    okCloudService = new lib::backend::OkCloudService(this);
    okCloudService->GetFederalInfo(
            [&](lib::backend::Res<lib::backend::FederalInfo>& res) {
                for (const auto& item : res.data->states) {
                    if (!item.xmppHost.isEmpty()) {
                        ui->providers->addItem(item.name);
                        m_hosts.push_back(item.xmppHost);
                        m_stacks.push_back(item.stackUrl);
                    }
                }

                if (ui->providers->count() > 1) {
                    auto& setting = lib::settings::OkSettings::getInstance();
                    int index = ui->providers->findText(setting.getProvider());
                    // found
                    if (index != -1) {
                        ui->providers->setCurrentIndex(index);
                    } else {
                        qDebug() << "provide not found in the comboBox: " << setting.getProvider();
                        ui->providers->setCurrentIndex(1);
                    }

                    m_loaded++;
                }
            },
            [&](int code, const QString& error) { onError(code, error); });

    // 4. UI
    ui->signUp->setCursor(Qt::PointingHandCursor);
    ui->signUp->installEventFilter(this);

    ui->findPwd->setCursor(Qt::PointingHandCursor);
    ui->findPwd->installEventFilter(this);

    ui->loginBtn->setCursor(Qt::PointingHandCursor);
}

void LoginWidget::deinit() {}

void LoginWidget::doLogin() {
    if (m_error) return;

    if (m_loaded < 1) {
        m_currentOriginalMsg = "Please waiting the page is loaded";
        setMsg(tr("Please waiting the page is loaded"));
        return;
    }

    // 获取服务提供商
    auto providerIdx = ui->providers->currentIndex();
    if (!(providerIdx > 0)) {
        m_currentOriginalMsg = "Please select service provider";
        setMsg(tr("Please select service provider"));
        return;
    }

    // 对登录时状态判断
    auto status = session->status();
    switch (status) {
        case lib::session::Status::SUCCESS: {
            qDebug(("SUCCESS ..."));
            return;
        }
        case lib::session::Status::CONNECTING: {
            qDebug(("CONNECTING ..."));
            return;
        }
        default: {
            QString host = m_hosts.at(providerIdx - 1);
            qDebug() << "Select provider host:" << host;

            // 登陆
            QString account(ui->accountInput->text());
            QString password(ui->passwordInput->text());
            // 对账号和密码判断
            if (account.isEmpty()) {
                qWarning() << "account is empty!";
                return;
            }
            if (ui->rember->isCheckable()) {
                m_settingManager->saveAccount(account, password);
            } else {
                m_settingManager->clearAccount();
            }

            SignInInfo info = {.account = account,
                               .password = password,
                               .host = host,
                               .stackUrl = m_stacks.at(providerIdx - 1)};
            session->doLogin(info);
            break;
        }
    }
}

void LoginWidget::onLoginResult(lib::session::SignInInfo info, lib::session::LoginResult result) {
    qDebug() << __func__ << "result=> " << result.msg;

    switch (result.status) {
        case lib::session::Status::NONE:
            break;
        case lib::session::Status::CONNECTING: {
            ui->loginMessage->setText(tr("..."));
            ui->loginBtn->setText(tr("Logging in"));
            QString account(ui->accountInput->text());
            QString password(ui->passwordInput->text());
            emit loginFailed(account, password);
            break;
        }
        case lib::session::Status::SUCCESS: {
            ui->loginMessage->setText(tr("login success"));
            QString account(ui->accountInput->text());
            QString password(ui->passwordInput->text());
            emit loginSuccess(account, password);
            break;
        }
        case lib::session::Status::FAILURE:
            ui->loginBtn->setText(tr("Login"));
            onError(result.statusCode, result.msg);
            break;
    }
}

void LoginWidget::on_loginBtn_released() {
    doLogin();
}

/**
 * 语言选择事件
 */
void LoginWidget::on_language_currentIndexChanged(int index) {
    qDebug() << "Select language:" << index;
    if (index == 0) {
        return;
    }

    auto& s = lib::settings::OkSettings::getInstance();
    const QString& locale = s.getLocales().at(index - 1);
    s.setTranslation(locale);
    s.saveGlobal();
    qDebug() << "Selected locale:" << locale;

    settings::Translator::translate(OK_UILoginWindow_MODULE, locale);
    qDebug() << "Translated locale:" << locale;
}

/**
 * 服务提供者事件
 */
void LoginWidget::on_providers_currentIndexChanged(int index) {
    qDebug() << "providers currentIndexChanged : " << index;
    if (index <= 0) {
        qDebug() << "provider index illegal";
        return;
    }

    auto& setting = lib::settings::OkSettings::getInstance();
    QString provider = ui->providers->currentText();
    QString settingProvider = setting.getProvider();

    qDebug() << "setting provider: " << settingProvider << " wanna change to: " << provider;

    if (settingProvider == provider) {
        qDebug() << "already in provider: " << provider;
        return;
    }
    setting.setProvider(provider);
    setting.saveGlobal();
    qDebug() << "change to provider:" << provider;
}

void LoginWidget::retranslateUi() {
    ui->retranslateUi(this);
    QString translatedMessage = tr(m_currentOriginalMsg.toUtf8().constData());
    setMsg(translatedMessage);
}

void LoginWidget::onError(int statusCode, const QString& msg) {
    QString newMsg = msg;
    switch (statusCode / 100) {
        case 0: {
            newMsg = tr("Network is not available!");
            m_currentOriginalMsg = "Network is not available!";
            break;
        }
        case 5: {
            newMsg = tr("Server error, please try again later!");
            m_currentOriginalMsg = "Server error, please try again later!";
            break;
        }
    }

    setMsg(newMsg);
    m_timer.reset();
}

void LoginWidget::setMsg(const QString& msg) {
    ui->loginMessage->setText(msg);
}

bool LoginWidget::eventFilter(QObject* obj, QEvent* event) {
    switch (event->type()) {
        case QEvent::MouseButtonPress: {
            if (obj == ui->settings) {
                showSettingDialog();
                break;
            }

            auto providerIdx = ui->providers->currentIndex();
            // validate
            if (providerIdx <= 0 || m_stacks.size() <= 0) {
                qWarning() << "providerIdx is illegal or servers is null";
                break;
            }
            QString host = m_stacks.at(providerIdx - 1);
            qDebug() << "Select provider host:" << host;

            if (obj == ui->signUp) {
                QDesktopServices::openUrl(QUrl(host + "/auth/register"));
            } else if (obj == ui->findPwd) {
                QDesktopServices::openUrl(QUrl(host + "/auth/forgot"));
            }

            break;
        }
        default:
            break;
    };
    return QObject::eventFilter(obj, event);
}

void LoginWidget::showEvent(QShowEvent* e) {}

void LoginWidget::showSettingDialog() {
    qDebug() << __func__;
}

void LoginWidget::onTimeout() {
    if (ui->rember->isChecked() && ui->providers->count() > 0) {
        if (!ui->passwordInput->text().isEmpty() && !ui->accountInput->text().isEmpty()) {
            on_loginBtn_released();
        }
    }
}

}  // namespace UI
