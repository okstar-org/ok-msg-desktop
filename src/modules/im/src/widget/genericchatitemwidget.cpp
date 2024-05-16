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
#include "maskablepixmapwidget.h"
#include "src/persistence/settings.h"
#include "src/widget/style.h"
#include "src/widget/tool/croppinglabel.h"
#include <QVariant>
#include <src/core/core.h>
#include <src/friendlist.h>
#include "src/model/friend.h"

GenericChatItemWidget::GenericChatItemWidget(ChatType type, const ContactId &cid, QWidget* parent)
    : QFrame(parent)
    , chatType(type), statusPic{nullptr}, contactId{cid}, prevStatus{Status::Status::None}
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

  // avatar
  QSize size;
//    if (isCompact())
//        size = QSize(20, 20);
//    else
      size = QSize(40, 40);

  avatar = new MaskablePixmapWidget(this, size, ":/img/avatar_mask.svg");
  statusPic = new QLabel(this);


  if(type == ChatType::Chat){
      avatar->setPixmap(QPixmap(":/img/contact.svg"));
      statusPic->setPixmap(QPixmap(Status::getIconPath(Status::Status::Offline)));
  }else{
      avatar->setPixmap(QPixmap(":/img/group.svg"));
  }
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

void GenericChatItemWidget::updateLastMessage(const Message &m)
{
    QString prefix;
    auto core = Core::getInstance();
    if(m.isGroup){
        //群聊显示前缀，单聊不显示
        if(ContactId(m.from).username == core->getUsername()){
            prefix = tr("I:");
        }else{
            auto f = FriendList::findFriend(ContactId(m.from));
            if(f){
                prefix=f->getDisplayedName()+tr(":");
            }else{
                prefix=m.displayName+tr(":");
            }
        }
    }
    setLastMessage(prefix+m.content);
}

void GenericChatItemWidget::updateStatusLight(Status::Status status, bool event)
{
    if(event && !isGroup()){
        //事件（通知）状态，保留当前状态
        auto f = FriendList::findFriend(contactId);
        if(f){
            prevStatus = f->getStatus();
        }
    }
    statusPic->setMargin(event ? 1 : 3);
    statusPic->setPixmap(QPixmap(Status::getIconPath(status, event)));
}

void GenericChatItemWidget::clearStatusLight()
{
    statusPic->clear();
    if(prevStatus!=Status::Status::None){
        //恢复之前状态
        statusPic->setMargin( 1 );
        statusPic->setPixmap(QPixmap(Status::getIconPath(prevStatus, false)));
    }
}
