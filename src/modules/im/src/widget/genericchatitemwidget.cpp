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
#include "src/model/group.h"



GenericChatItemWidget::GenericChatItemWidget(ChatType type, const ContactId &cid, QWidget* parent)
    : QFrame(parent)
    , chatType(type), statusPic{nullptr},
      contactId{cid}, prevStatus{Status::Status::None},
      active{false}
{
    nameLabel = new CroppingLabel(this);
    nameLabel->setTextFormat(Qt::PlainText);

    lastMessageLabel = new CroppingLabel(this);
    lastMessageLabel->setTextFormat(Qt::PlainText);
    lastMessageLabel->setText("");
  auto p = lastMessageLabel->palette();
  p.setColor(QPalette::WindowText, Style::getColor(Style::GroundExtra));

  auto newFont= lastMessageLabel->font();
  newFont.setPixelSize(newFont.pixelSize()*.7);

  lastMessageLabel->setFont(newFont);
  lastMessageLabel->setPalette(p);
//  lastMessageLabel->setForegroundRole(QPalette::WindowText);

  statusPic = new QLabel(this);
  if(type == ChatType::Chat){ 
      statusPic->setPixmap(QPixmap(Status::getIconPath(Status::Status::Offline)));
  }


  QSize size = QSize(40, 40);
  avatar = new MaskablePixmapWidget(this, size, ":/img/avatar_mask.svg");



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
    if(!isGroup()){
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



bool GenericChatItemWidget::isActive()
{
    return active;
}

void GenericChatItemWidget::setActive(bool _active)
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

//    if(avatarSetStatus == Status::AvatarSet::DefaultSet){
//        setDefaultAvatar();
//    }

    onActiveSet(active);
}

void GenericChatItemWidget::setAvatar(const QPixmap &pic)
{
    qDebug() << __func__ << "pic:" << pic;
    if(pic.isNull()){
        return;
    }

    avatar->setPixmap(pic);

}

void GenericChatItemWidget::clearAvatar()
{
    avatar->clear();

    auto c =getContact();
    if(c){
      c->clearAvatar();
    }
}

void GenericChatItemWidget::setDefaultAvatar()
{
    qDebug() << __func__;

    auto c = getContact();

        auto pix= c->setDefaultAvatar();
        avatar->setPixmap(pix);


}

void GenericChatItemWidget::showEvent(QShowEvent *e)
{
    Contact* c = getContact();

      avatar->setPixmap(c->getAvatar());
      nameLabel->setText(c->getDisplayedName());

}

