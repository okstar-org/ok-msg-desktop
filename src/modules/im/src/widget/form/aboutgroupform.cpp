#include "aboutgroupform.h"


#include <QDebug>
#include "ui_aboutgroupform.h"
#include "src/grouplist.h"
#include "src/model/group.h"

AboutGroupForm::AboutGroupForm(const GroupId& groupId_, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AboutGroupForm),
    groupId{groupId_}
{
    ui->setupUi(this);
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

    auto name = group->getDisplayedName();
    ui->name->setText(name);

    auto map = group->getPeerList();
    for(auto peer : map){
        auto f = new QLabel();
        f->setText(peer);
        ui->friendListLayout->addWidget(f);
    }
}
