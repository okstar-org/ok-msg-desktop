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
#include <QIcon>
#include <QSvgRenderer>
#include <QVariant>
#include "lib/ui/widget/tools/CroppingLabel.h"
#include "src/core/core.h"
#include "src/lib/storage/settings/style.h"
#include "src/lib/ui/widget/tools/MaskablePixmap.h"
#include "src/model/friend.h"
#include "src/model/friendlist.h"
#include "src/model/group.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"

namespace module::im {

GenericChatItemWidget::GenericChatItemWidget(ChatType type,
                                             const ContactId& cid,
                                             QWidget* parent)
        : QFrame(parent)
        , chatType(type)
        , statusPic{nullptr}
        , contactId{cid}
        , contact{nullptr}
        , prevStatus{Status::None}
        , active{false}
        , showContextMenu{true} {
    nameLabel = new lib::ui::CroppingLabel(this);
    nameLabel->setObjectName("nameLabel");
    nameLabel->setTextFormat(Qt::PlainText);
    nameLabel->setText(cid.username);

    lastMessageLabel = new lib::ui::CroppingLabel(this);
    lastMessageLabel->setObjectName("lastMessageLabel");
    lastMessageLabel->setTextFormat(Qt::PlainText);
    lastMessageLabel->setText("");

    auto p = lastMessageLabel->palette();
    p.setColor(QPalette::WindowText,
               lib::settings::Style::getColor(lib::settings::Style::ColorPalette::GroundExtra));

    auto newFont = lastMessageLabel->font();
    newFont.setPixelSize(newFont.pixelSize() * .7);

    lastMessageLabel->setFont(newFont);
    lastMessageLabel->setPalette(p);
    //  lastMessageLabel->setForegroundRole(QPalette::WindowText);

    statusPic = new QLabel(this);
    statusPic->setContentsMargins(1, 1, 1, 1);
    if (type == ChatType::Chat) {
        updateStatusLight(Status::Offline, false);
    } else {
        clearStatusLight();
    }

    avatar = new lib::ui::MaskablePixmapWidget(this, QSize(40, 40), ":/img/avatar_mask.svg");
    auto profile = Nexus::getProfile();
    auto avt = profile->loadAvatar(contactId);
    if (!avt.isNull()) {
        avatar->setPixmap(avt);
    } else {
        setDefaultAvatar();
    }
}

GenericChatItemWidget::~GenericChatItemWidget() {
    qDebug() << __func__;
}

QString GenericChatItemWidget::getName() const {
    return nameLabel->fullText();
}

void GenericChatItemWidget::setName(const QString& name) {
    nameLabel->setText(name);
}

void GenericChatItemWidget::searchName(const QString& searchString, bool hide) {
    setVisible(!hide && getName().contains(searchString, Qt::CaseInsensitive));
}

void GenericChatItemWidget::setLastMessage(const QString& msg) {
    if (msg.contains(QChar('\n')))
        lastMessageLabel->setText(QString(msg).replace(QChar('\n'), QChar(' ')));
    else
        lastMessageLabel->setText(msg);
}

void GenericChatItemWidget::updateLastMessage(const Message& m) {
    QString prefix;
    auto core = Core::getInstance();
    if (m.isGroup) {
        // 群聊显示前缀，单聊不显示
        if (ContactId(m.from).username == core->getUsername()) {
            prefix = tr("I:");
        } else {
            auto f = Nexus::getCore()->getFriendList().findFriend(ContactId(m.from));
            if (f) {
                prefix = f->getDisplayedName() + tr(":");
            } else {
                prefix = m.displayName + tr(":");
            }
        }
    }
    setLastMessage(prefix + m.content);
}

void GenericChatItemWidget::updateStatusLight(Status status, bool event) {
    if (!statusPic) return;

    auto pix = getIconPath(status, event);
    if (pix.isEmpty()) return;

    // 图片是svg格式，按照原有逻辑先获取默认尺寸
    QSvgRenderer svgrender(pix);
    QSize s = svgrender.defaultSize();
    statusPic->setPixmap(QIcon(pix).pixmap(this->window()->windowHandle(), s));
}

void GenericChatItemWidget::clearStatusLight() {
    statusPic->clear();
}

bool GenericChatItemWidget::isActive() {
    return active;
}

void GenericChatItemWidget::setActive(bool _active) {
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

    //    if(avatarSetStatus == AvatarSet::DefaultSet){
    //        setDefaultAvatar();
    //    }

    onActiveSet(active);
}

void GenericChatItemWidget::setAvatar(const QPixmap& pic) {
    if (pic.isNull()) {
        return;
    }
    avatar->setPixmap(pic);
}

void GenericChatItemWidget::clearAvatar() {
    qDebug() << __func__;
    avatar->clear();
}

void GenericChatItemWidget::setDefaultAvatar() {
    qDebug() << __func__;
    auto name = (chatType == ChatType::Chat) ? "contact" : "group";
    auto uri = QString(":img/%1_dark.svg").arg(name);
    avatar->setPixmap(QPixmap(uri));
}

void GenericChatItemWidget::setContact(const Contact& contact_) {
    contact = &contact_;
    if (contact) {
        setName(contact->getDisplayedName());
        setAvatar(contact->getAvatar());
    }
}

void GenericChatItemWidget::removeContact() {
    qDebug() << __func__;
    contact = nullptr;
}

void GenericChatItemWidget::showEvent(QShowEvent* e) {
    if (contact) {
        setName(contact->getDisplayedName());
        setAvatar(contact->getAvatar());
    }
}
}  // namespace module::im
