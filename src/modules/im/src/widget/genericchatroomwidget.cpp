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
#include "maskablepixmapwidget.h"
#include "src/persistence/settings.h"
#include "src/widget/style.h"
#include "src/widget/tool/croppinglabel.h"
#include <QBoxLayout>
#include <QMouseEvent>

GenericChatroomWidget::GenericChatroomWidget(ChatType type,  const ContactId &cid,  QWidget* parent)
    : GenericChatItemWidget(type, cid, parent),
      contactId(cid), active{false}
{

    Settings& s = Settings::getInstance();
    connect(&s, &Settings::compactLayoutChanged, this, &GenericChatroomWidget::compactChange);

    setAutoFillBackground(true);
    reloadTheme();

    compactChange(false);
}

bool GenericChatroomWidget::eventFilter(QObject*, QEvent*)
{
    return true; // Disable all events.
}

void GenericChatroomWidget::compactChange(bool _compact)
{
//    if (!isCompact())
//        delete textLayout; // has to be first, deleted by layout

    setCompact(_compact);

    delete mainLayout;

    mainLayout = new QHBoxLayout;
    textLayout = new QVBoxLayout;

    setLayout(mainLayout);
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);
    textLayout->setSpacing(0);
    textLayout->setMargin(0);
    setLayoutDirection(Qt::LeftToRight); // parent might have set Qt::RightToLeft

    // avatar
    if (isCompact()) {
        delete textLayout; // Not needed
        setFixedHeight(25);
        avatar->setSize(QSize(20, 20));
        mainLayout->addSpacing(18);
        mainLayout->addWidget(avatar);
        mainLayout->addSpacing(5);
        mainLayout->addWidget(nameLabel);
        mainLayout->addWidget(lastMessageLabel);
        mainLayout->addSpacing(5);

        if(statusPic){
            mainLayout->addWidget(statusPic);
            mainLayout->addSpacing(5);
        }

        mainLayout->activate();
//        statusMessageLabel->setFont(Style::getFont(Style::Small));
        nameLabel->setFont(Style::getFont(Style::Medium));
    } else {
        setFixedHeight(55);
        avatar->setSize(QSize(40, 40));
        textLayout->addStretch();
        textLayout->addWidget(nameLabel);
        textLayout->addWidget(lastMessageLabel);
        textLayout->addStretch();
        mainLayout->addSpacing(20);
        mainLayout->addWidget(avatar);
        mainLayout->addSpacing(10);
        mainLayout->addLayout(textLayout);
        mainLayout->addSpacing(10);
        if(statusPic){
        mainLayout->addWidget(statusPic);
        mainLayout->addSpacing(10);}
        mainLayout->activate();
//        statusMessageLabel->setFont(Style::getFont(Style::Medium));
        nameLabel->setFont(Style::getFont(Style::Big));
    }
}

bool GenericChatroomWidget::isActive()
{
    return active;
}

void GenericChatroomWidget::setActive(bool _active)
{
    active = _active;
    if (active) {
        setBackgroundRole(QPalette::Highlight);
//        statusMessageLabel->setForegroundRole(QPalette::HighlightedText);
        nameLabel->setForegroundRole(QPalette::HighlightedText);
    } else {
        setBackgroundRole(QPalette::Window);
//        statusMessageLabel->setForegroundRole(QPalette::WindowText);
        nameLabel->setForegroundRole(QPalette::WindowText);
    }

    onSetActive(active);
}

void GenericChatroomWidget::setName(const QString& name)
{
    nameLabel->setText(name);
}

void GenericChatroomWidget::setStatusMsg(const QString& status)
{
//    statusMessageLabel->setText(status);
}

QString GenericChatroomWidget::getStatusMsg() const
{
//    return statusMessageLabel->text();
    return QString{};
}

QString GenericChatroomWidget::getTitle() const
{
    QString title = getName();

    if (!getStatusString().isNull())
        title += QStringLiteral(" (") + getStatusString() + QStringLiteral(")");

    return title;
}

void GenericChatroomWidget::reloadTheme()
{
    QPalette p;

//    p = statusMessageLabel->palette();
//    p.setColor(QPalette::WindowText, Style::getColor(Style::GroundExtra));       // Base color
//    p.setColor(QPalette::HighlightedText, Style::getColor(Style::StatusActive)); // Color when active
//    statusMessageLabel->setPalette(p);

    p = nameLabel->palette();
    p.setColor(QPalette::WindowText, Style::getColor(Style::MainText));           // Base color
    p.setColor(QPalette::HighlightedText, Style::getColor(Style::NameActive)); // Color when active
    nameLabel->setPalette(p);

    p = palette();
    p.setColor(QPalette::Window, Style::getColor(Style::ThemeMedium));   // Base background color
    p.setColor(QPalette::Highlight, Style::getColor(Style::ThemeHighlight)); // On mouse over
    p.setColor(QPalette::Light, Style::getColor(Style::ThemeLight));          // When active
    setPalette(p);


}

void GenericChatroomWidget::activate()
{
    emit chatroomWidgetClicked(this);
}

void GenericChatroomWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit chatroomWidgetClicked(this);
    } else if (event->button() == Qt::MiddleButton) {
        emit middleMouseClicked();
    } else {
        event->ignore();
    }
}

void GenericChatroomWidget::enterEvent(QEvent*)
{
    if (!active)
        setBackgroundRole(QPalette::Light);
}

void GenericChatroomWidget::leaveEvent(QEvent* event)
{
    if (!active)
        setBackgroundRole(QPalette::Window);
    QWidget::leaveEvent(event);
}
