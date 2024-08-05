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

#include "groupinvitewidget.h"

#include "src/core/core.h"
#include "src/nexus.h"
#include "src/persistence/settings.h"
#include "src/widget/tool/croppinglabel.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QSignalMapper>

/**
 * @class GroupInviteWidget
 *
 * @brief This class shows information about single group invite
 * and provides buttons to accept/reject it
 */

GroupInviteWidget::GroupInviteWidget(QWidget* parent, const GroupInvite& invite)
        : QWidget(parent)
        , acceptButton(new QPushButton(this))
        , rejectButton(new QPushButton(this))
        , inviteMessageLabel(new CroppingLabel(this))
        , widgetLayout(new QHBoxLayout(this))
        , inviteInfo(invite) {
    connect(acceptButton, &QPushButton::clicked, [this]() { emit accepted(inviteInfo); });
    connect(rejectButton, &QPushButton::clicked, [this]() { emit rejected(inviteInfo); });
    widgetLayout->addWidget(inviteMessageLabel);
    widgetLayout->addWidget(acceptButton);
    widgetLayout->addWidget(rejectButton);
    setLayout(widgetLayout);
    retranslateUi();
}

/**
 * @brief Retranslate all elements in the form.
 */
void GroupInviteWidget::retranslateUi() {
    QString name = Nexus::getCore()->getFriendUsername(inviteInfo.getFriendId());
    QDateTime inviteDate = inviteInfo.getInviteDate();
    QString date = inviteDate.toString(Settings::getInstance().getDateFormat());
    QString time = inviteDate.toString(Settings::getInstance().getTimestampFormat());

    inviteMessageLabel->setText(tr("Invited by %1 on %2 at %3.")
                                        .arg("<b>%1</b>")
                                        .arg(name.toHtmlEscaped(), date, time));
    acceptButton->setText(tr("Join"));
    rejectButton->setText(tr("Decline"));
}

/**
 * @brief Returns infomation about invitation - e.g., who and when sent
 * @return Invite information object
 */
const GroupInvite GroupInviteWidget::getInviteInfo() const { return inviteInfo; }
