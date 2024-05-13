#include "aboutgroupform.h"


#include <QDebug>
#include "ui_aboutgroupform.h"
#include "src/grouplist.h"
#include "src/model/group.h"
#include "src/widget/widget.h"


AboutGroupForm::AboutGroupForm(const GroupId& groupId_, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AboutGroupForm),
    groupId{groupId_}
{
    ui->setupUi(this);

    connect(ui->sendMessage, &QPushButton::clicked, this, &AboutGroupForm::onSendMessageClicked);
    init();
}

AboutGroupForm::~AboutGroupForm()
{
    delete ui;
}


void AboutGroupForm::init(){

    auto group =  GroupList::findGroup(groupId);
    if(!group){
        qWarning() << "Group"<< groupId.getUsername() <<"is no existing!";
        return;
    }

    connect(group, &Group::titleChanged, this, [&](const QString& author,const QString &title){
        ui->title->setText(title);
    });
    connect(group, &Group::peerCountChanged, this, [&](uint32_t count){
        ui->occupants->setText(QString::number(count));
    });

    connect(group, &Group::descChanged, this, [&](const QString& desc){
        ui->desc->setText(desc);
    });

    auto name = group->getDisplayedName();
    ui->name->setText(name);
    ui->occupants->setText(QString::number(group->getPeersCount()));
    ui->id->setText(group->getId());
    ui->desc->setText(group->getDesc());
    ui->title->setText(group->getTitle());

    auto map = group->getPeerList();
    for(auto peer : map){
        auto f = new QLabel();
        f->setText(peer);
        ui->friendListLayout->addWidget(f);
    }
}

void AboutGroupForm::onSendMessageClicked()
{
    auto widget = Widget::getInstance();
    if(widget){
      emit widget->toSendMessage(ui->id->text());
    }
}
