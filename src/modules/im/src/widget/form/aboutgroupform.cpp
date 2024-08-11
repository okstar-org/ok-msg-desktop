#include "aboutgroupform.h"

#include <src/nexus.h>
#include <QDebug>
#include "src/grouplist.h"
#include "src/model/group.h"
#include "src/persistence/profile.h"
#include "src/widget/widget.h"
#include "ui_aboutgroupform.h"

AboutGroupForm::AboutGroupForm(const GroupId& groupId_, QWidget* parent)
        : QWidget(parent)
        , ui(new Ui::AboutGroupForm)
        , groupId{groupId_}
        , group{GroupList::findGroup(groupId_)} {
    ui->setupUi(this);

    ui->id->setText(group->getId());
    ui->occupants->setText(QString::number(group->getPeersCount()));
    ui->avatar->setPixmap(group->getAvatar());

    ui->desc->setText(group->getDesc());
    connect(ui->desc, &QLineEdit::textChanged, this, &AboutGroupForm::doDescChanged);

    connect(group, &Group::descChanged, [&](auto desc) { ui->desc->setText(desc); });

    // 不可变(只接收来自服务器修改的推送)
    ui->groupName->setText(group->getName());
    connect(group, &Group::nameChanged, this, [&](const QString& name) {
        ui->groupName->setText(name);
        ui->name->setText(name);
    });

    // 名称
    ui->name->setText(group->getName());
    connect(ui->name, &QLineEdit::textChanged, this, &AboutGroupForm::doNameChanged);

    // 备注名
    ui->alias->setText(group->getDisplayedName());
    connect(ui->alias, &QLineEdit::textChanged, this, &AboutGroupForm::doAliasChanged);

    // 主题
    connect(ui->notice, &QLineEdit::textChanged, this, &AboutGroupForm::doSubjectChanged);
    ui->subject->setText(group->getSubject());
    connect(group, &Group::subjectChanged, this,
            [&](const QString& name) { ui->subject->setText(name); });

    connect(group, &Group::subjectChanged, this,
            [&](const QString& author, const QString& title) { ui->subject->setText(title); });
    connect(group, &Group::peerCountChanged, this,
            [&](uint32_t count) { ui->occupants->setText(QString::number(count)); });

    connect(group, &Group::descChanged, this,
            [&](const QString& desc) { ui->desc->setText(desc); });

    connect(group, &Group::privilegesChanged, this,
            [&](const Group::Role& role, const Group::Affiliation& aff, const QList<int> codes) {
                updateUI();
            });

    connect(ui->sendMessage, &QPushButton::clicked, this, &AboutGroupForm::onSendMessageClicked);

    auto map = group->getPeerList();
    for (auto peer : map) {
        auto f = new QLabel(peer);
        ui->friendListLayout->addWidget(f);
    }

    updateUI();
}

AboutGroupForm::~AboutGroupForm() { delete ui; }

void AboutGroupForm::updateUI() {
    auto role = group->getRole();
    if (role >= Group::Role::Participant) {
        ui->subject->setEnabled(true);
    } else {
        ui->subject->setDisabled(true);
    }

    auto aff = group->getAffiliation();
    if (aff == Group::Affiliation::Owner) {
        // https://xmpp.org/extensions/xep-0045.html#table-6
        // 具有权限：“Change Room Configuration”，
        // 涉及字段：https://xmpp.org/extensions/xep-0045.html#registrar-formtype-owner
        ui->name->setEnabled(true);
        ui->desc->setEnabled(true);
        ui->subject->setEnabled(true);
    } else {
        ui->name->setEnabled(false);
        ui->desc->setEnabled(false);
        ui->subject->setEnabled(false);
    }
}

void AboutGroupForm::onSendMessageClicked() {
    auto widget = Widget::getInstance();
    if (widget) {
        qDebug() << "toSendMessage:" << ui->id->text();
        emit widget->toSendMessage(ui->id->text(), true);
    }
}

void AboutGroupForm::doNameChanged(const QString& text) {
    qDebug() << __func__ << text;
    group->setName(text);
    Core::getInstance()->setGroupName(groupId.toString(), text);
}

void AboutGroupForm::doAliasChanged(const QString& text) {
    qDebug() << __func__ << text;
    //    auto profile = Nexus::getInstance().getProfile();
    //    profile->saveFriendAlias(groupId.toString(), text);
    group->setAlias(text);
    Core::getInstance()->setGroupAlias(groupId.toString(), text);
}

void AboutGroupForm::doSubjectChanged(const QString& text) {
    qDebug() << __func__ << text;
    group->setSubject({}, text);
    Core::getInstance()->setGroupSubject(groupId.toString(), text);
}

void AboutGroupForm::doDescChanged(const QString& text) {
    qDebug() << __func__ << text;
    group->setDesc(text);
    Core::getInstance()->setGroupDesc(groupId.toString(), text);
}
