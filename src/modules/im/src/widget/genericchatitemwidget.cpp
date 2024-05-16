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

#include "genericchatitemwidget.h"
#include "src/persistence/settings.h"
#include "src/widget/style.h"
#include "src/widget/tool/croppinglabel.h"
#include <QVariant>

GenericChatItemWidget::GenericChatItemWidget(ChatType type, QWidget* parent)
    : QFrame(parent)
    , chatType(type)
{
    nameLabel = new CroppingLabel(this);
    nameLabel->setTextFormat(Qt::PlainText);

    lastMessageLabel = new CroppingLabel(this);
    lastMessageLabel->setTextFormat(Qt::PlainText);
    lastMessageLabel->setText("");





  auto p = lastMessageLabel->palette();
  p.setColor(QPalette::WindowText, Style::getColor(Style::GroundExtra));
//  p.setColor(QPalette::HighlightedText, Style::getColor(Style::GroundExtra));
//  auto fs = nameLabel->font().pixelSize()*.8;

  auto newFont= lastMessageLabel->font();
  newFont.setPixelSize(newFont.pixelSize()*.7);

  lastMessageLabel->setFont(newFont);
  lastMessageLabel->setPalette(p);
//  lastMessageLabel->setForegroundRole(QPalette::WindowText);
}


QString GenericChatItemWidget::getName() const
{
    return nameLabel->fullText();
}

void GenericChatItemWidget::searchName(const QString& searchString, bool hide)
{
    setVisible(!hide && getName().contains(searchString, Qt::CaseInsensitive));
}

void GenericChatItemWidget::setLastMessage(const QString &msg)
{
    lastMessageLabel->setText(msg);
}
