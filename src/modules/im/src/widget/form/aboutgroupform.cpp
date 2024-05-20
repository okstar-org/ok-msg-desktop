#include "aboutgroupform.h"


#include <QDebug>
#include "ui_aboutgroupform.h"
#include "src/grouplist.h"
#include "src/model/group.h"
#include "src/widget/widget.h"


AboutGroupForm::AboutGroupForm(const GroupId& groupId_, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AboutGroupForm),
    groupId{groupId_}, group{GroupList::findGroup(groupId_)}
{
    ui->setupUi(this);

    connect(ui->sendMessage, &QPushButton::clicked, this, &AboutGroupForm::onSendMessageClicked);

    ui->id->setText(group->getId());
    ui->name->setText(group->getDisplayedName());
    ui->title->setText(group->getTitle());
    ui->desc->setText(group->getDesc());
    ui->occupants->setText(QString::number(group->getPeersCount()));

    init();
}

AboutGroupForm::~AboutGroupForm()
{
    delete ui;
}


void AboutGroupForm::init(){
    connect(group, &Group::titleChanged, this, [&](const QString& author,const QString &title){
        ui->title->setText(title);
    });
    connect(group, &Group::peerCountChanged, this, [&](uint32_t count){
        ui->occupants->setText(QString::number(count));
    });

    connect(group, &Group::descChanged, this, [&](const QString& desc){
        ui->desc->setText(desc);
    });

    connect(group, &Group::privilegesChanged, this, [&](const Group::Role &role, const Group::Affiliation &aff, const QList<int> codes){
        updateUI();
    });

    auto map = group->getPeerList();
    for(auto peer : map){
        auto f = new QLabel(peer);
        ui->friendListLayout->addWidget(f);
    }

    updateUI();
}

void AboutGroupForm::updateUI()
{
    auto role= group->getRole();
    if(role >= Group::Role::Participant){
        ui->title->setEnabled(true);
    }else{
        ui->title->setDisabled(true);
    }
}

void AboutGroupForm::onSendMessageClicked()
{
    auto widget = Widget::getInstance();
    if(widget){
        qDebug() <<"toSendMessage:"<< ui->id->text();
      emit widget->toSendMessage(ui->id->text(), true);
    }
}
