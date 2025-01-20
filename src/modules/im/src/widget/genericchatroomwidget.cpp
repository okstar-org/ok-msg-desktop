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

#include "genericchatroomwidget.h"
#include <QBoxLayout>
#include <QMouseEvent>
#include "lib/storage/settings/style.h"
#include "lib/ui/widget/tools/CroppingLabel.h"
#include "src/lib/ui/widget/tools/MaskablePixmap.h"
namespace module::im {

GenericChatroomWidget::GenericChatroomWidget(ChatType type, const ContactId& cid, QWidget* parent)
        : GenericChatItemWidget(type, cid, parent), contactId(cid) {
    setAutoFillBackground(true);
    reloadTheme();
    compactChange(false);
}

GenericChatroomWidget::~GenericChatroomWidget() {}

bool GenericChatroomWidget::eventFilter(QObject*, QEvent*) {
    return true;  // Disable all events.
}

void GenericChatroomWidget::compactChange(bool _compact) {
    //    if (!isCompact())
    //        delete textLayout; // has to be first, deleted by layout

    setCompact(_compact);

    delete mainLayout;

    mainLayout = new QHBoxLayout;
    textLayout = new QVBoxLayout;

    setLayout(mainLayout);
    textLayout->setSpacing(0);
    textLayout->setMargin(0);
    setLayoutDirection(Qt::LeftToRight);  // parent might have set Qt::RightToLeft

    // avatar
    if (isCompact()) {
        mainLayout->setContentsMargins(5, 5, 5, 5);
        mainLayout->setSpacing(5);
        delete textLayout;  // Not needed
        avatar->setSize(QSize(20, 20));
        mainLayout->addWidget(avatar);
        mainLayout->addWidget(nameLabel);
        mainLayout->addWidget(lastMessageLabel);
        if (statusPic) {
            mainLayout->addWidget(statusPic);
        }
        mainLayout->activate();
        nameLabel->setFont(lib::settings::Style::getFont(lib::settings::Style::Font::Medium));
    } else {
        mainLayout->setContentsMargins(8, 8, 8, 8);
        mainLayout->setSpacing(10);
        avatar->setSize(QSize(40, 40));
        textLayout->addStretch();
        textLayout->addWidget(nameLabel);
        textLayout->addWidget(lastMessageLabel);
        textLayout->addStretch();
        mainLayout->addWidget(avatar);
        mainLayout->addLayout(textLayout);
        if (statusPic) {
            mainLayout->addWidget(statusPic);
        }
        mainLayout->activate();
        nameLabel->setFont(lib::settings::Style::getFont(lib::settings::Style::Font::Big));
    }
}

void GenericChatroomWidget::setName(const QString& name) {
    GenericChatItemWidget::setName(name);
    nameLabel->setText(name);
}

void GenericChatroomWidget::setStatusMsg(const QString& status) {
    //    statusMessageLabel->setText(status);
}

QString GenericChatroomWidget::getStatusMsg() const {
    //    return statusMessageLabel->text();
    return QString{};
}

QString GenericChatroomWidget::getTitle() const {
    QString title = getName();

    if (!getStatusString().isNull())
        title += QStringLiteral(" (") + getStatusString() + QStringLiteral(")");

    return title;
}

void GenericChatroomWidget::reloadTheme() {
    QPalette p;

    //    p = statusMessageLabel->palette();
    //    p.setColor(QPalette::WindowText, Style::getColor(Style::GroundExtra));       // Base color
    //    p.setColor(QPalette::HighlightedText, Style::getColor(Style::StatusActive)); // Color when
    //    active statusMessageLabel->setPalette(p);

    p = nameLabel->palette();
    p.setColor(QPalette::WindowText,
               lib::settings::Style::getColor(
                       lib::settings::Style::ColorPalette::MainText));  // Base color
    p.setColor(QPalette::HighlightedText,
               lib::settings::Style::getColor(
                       lib::settings::Style::ColorPalette::NameActive));  // Color when active
    nameLabel->setPalette(p);

    p = palette();
    p.setColor(QPalette::Window,
               lib::settings::Style::getColor(
                       lib::settings::Style::ColorPalette::ThemeMedium));  // Base background color
    p.setColor(QPalette::Highlight,
               lib::settings::Style::getColor(
                       lib::settings::Style::ColorPalette::ThemeHighlight));  // On mouse over
    p.setColor(QPalette::Light,
               lib::settings::Style::getColor(
                       lib::settings::Style::ColorPalette::ThemeLight));  // When active
    setPalette(p);
}

void GenericChatroomWidget::activate() {
    emit chatroomWidgetClicked(this);
}

void GenericChatroomWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit chatroomWidgetClicked(this);
    } else if (event->button() == Qt::MiddleButton) {
        emit middleMouseClicked();
    } else {
        event->ignore();
    }
}

void GenericChatroomWidget::enterEvent(QEvent*) {
    if (!active) setBackgroundRole(QPalette::Light);
}

void GenericChatroomWidget::leaveEvent(QEvent* event) {
    if (!active) setBackgroundRole(QPalette::Window);
    QWidget::leaveEvent(event);
}
}  // namespace module::im