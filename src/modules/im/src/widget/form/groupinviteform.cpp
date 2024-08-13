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

#include "groupinviteform.h"

#include "lib/settings/translator.h"
#include "src/core/core.h"
#include "src/model/groupinvite.h"
#include "src/persistence/settings.h"
#include "src/widget/contentlayout.h"
#include "src/widget/form/groupinvitewidget.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QDebug>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSignalMapper>
#include <QVBoxLayout>
#include <QWindow>

#include <algorithm>

/**
 * @class GroupInviteForm
 *
 * @brief This form contains all group invites you received
 */

GroupInviteForm::GroupInviteForm()
        : headWidget(new QWidget(this))
        , headLabel(new QLabel(this))
        , createButton(new QPushButton(this))
        , inviteBox(new QGroupBox(this))
        , scroll(new QScrollArea(this)) {
    QVBoxLayout* layout = new QVBoxLayout(this);
    connect(createButton, &QPushButton::clicked,
            [this]() { emit groupCreate(ConferenceType::AV); });

    QWidget* innerWidget = new QWidget(scroll);
    innerWidget->setLayout(new QVBoxLayout());
    innerWidget->layout()->setAlignment(Qt::AlignTop);
    scroll->setWidget(innerWidget);
    scroll->setWidgetResizable(true);

    QVBoxLayout* inviteLayout = new QVBoxLayout(inviteBox);
    inviteLayout->addWidget(scroll);

    layout->addWidget(createButton);
    layout->addWidget(inviteBox);

    QFont bold;
    bold.setBold(true);

    headLabel->setFont(bold);
    QHBoxLayout* headLayout = new QHBoxLayout(headWidget);
    headLayout->addWidget(headLabel);

    retranslateUi();
    settings::Translator::registerHandler(std::bind(&GroupInviteForm::retranslateUi, this), this);
}

GroupInviteForm::~GroupInviteForm() { settings::Translator::unregister(this); }

/**
 * @brief Detects that form is shown
 * @return True if form is visible
 */
bool GroupInviteForm::isShown() const {
    bool result = isVisible();
    if (result) {
        headWidget->window()->windowHandle()->alert(0);
    }
    return result;
}

/**
 * @brief Shows the form
 * @param contentLayout Main layout that contains all components of the form
 */
void GroupInviteForm::show(ContentLayout* contentLayout) {
    //    contentLayout->mainContent->layout()->addWidget(this);
    //    contentLayout->mainHead->layout()->addWidget(headWidget);
    QWidget::show();
    headWidget->show();
}

/**
 * @brief Adds group invite
 * @param inviteInfo Object which contains info about group invitation
 * @return true if notification is needed, false otherwise
 */
bool GroupInviteForm::addGroupInvite(const GroupInvite& inviteInfo) {
    // supress duplicate invite messages
    for (GroupInviteWidget* existing : invites) {
        if (existing->getInviteInfo().getInvite() == inviteInfo.getInvite()) {
            return false;
        }
    }

    GroupInviteWidget* widget = new GroupInviteWidget(this, inviteInfo);
    scroll->widget()->layout()->addWidget(widget);
    invites.append(widget);
    connect(widget, &GroupInviteWidget::accepted, [this](const GroupInvite& inviteInfo) {
        deleteInviteWidget(inviteInfo);
        emit groupInviteAccepted(inviteInfo);
    });

    connect(widget, &GroupInviteWidget::rejected,
            [this](const GroupInvite& inviteInfo) { deleteInviteWidget(inviteInfo); });
    if (isVisible()) {
        emit groupInvitesSeen();
        return false;
    }
    return true;
}

void GroupInviteForm::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    emit groupInvitesSeen();
}

/**
 * @brief Deletes accepted/declined group invite widget
 * @param inviteInfo Invite information of accepted/declined widget
 */
void GroupInviteForm::deleteInviteWidget(const GroupInvite& inviteInfo) {
    auto deletingWidget = std::find_if(
            invites.begin(), invites.end(),
            [&](const GroupInviteWidget* widget) { return inviteInfo == widget->getInviteInfo(); });
    (*deletingWidget)->deleteLater();
    scroll->widget()->layout()->removeWidget(*deletingWidget);
    invites.erase(deletingWidget);
}

void GroupInviteForm::retranslateUi() {
    headLabel->setText(tr("Groups"));
    if (createButton) {
        createButton->setText(tr("Create new group"));
    }
    inviteBox->setTitle(tr("Group invites"));
    for (GroupInviteWidget* invite : invites) {
        invite->retranslateUi();
    }
}
