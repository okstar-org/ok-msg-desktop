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

#include "filesform.h"
#include <QFileInfo>
#include <QWindow>
#include "lib/settings/translator.h"
#include "src/lib/settings/style.h"
#include "src/widget/contentlayout.h"
#include "src/widget/widget.h"

FilesForm::FilesForm()
        : QObject(), doneIcon(Style::getImagePath("fileTransferWidget/fileDone.svg")) {
    head = new QWidget();
    QFont bold;
    bold.setBold(true);
    headLabel.setFont(bold);
    head->setLayout(&headLayout);
    headLayout.addWidget(&headLabel);

    recvd = new QListWidget;
    sent = new QListWidget;

    main.addTab(recvd, QString());
    main.addTab(sent, QString());

    connect(sent, &QListWidget::itemActivated, this, &FilesForm::onFileActivated);
    connect(recvd, &QListWidget::itemActivated, this, &FilesForm::onFileActivated);

    retranslateUi();
    settings::Translator::registerHandler(std::bind(&FilesForm::retranslateUi, this), this);
}

FilesForm::~FilesForm() {
    settings::Translator::unregister(this);
    delete recvd;
    delete sent;
    head->deleteLater();
}

bool FilesForm::isShown() const {
    if (main.isVisible()) {
        head->window()->windowHandle()->alert(0);
        return true;
    }

    return false;
}

void FilesForm::show(ContentLayout* contentLayout) {
    //    contentLayout->mainContent->layout()->addWidget(&main);
    //    contentLayout->mainHead->layout()->addWidget(head);
    main.show();
    head->show();
}

void FilesForm::onFileDownloadComplete(const QString& path) {
    QListWidgetItem* tmp = new QListWidgetItem(doneIcon, QFileInfo(path).fileName());
    tmp->setData(Qt::UserRole, path);
    recvd->addItem(tmp);
}

void FilesForm::onFileUploadComplete(const QString& path) {
    QListWidgetItem* tmp = new QListWidgetItem(doneIcon, QFileInfo(path).fileName());
    tmp->setData(Qt::UserRole, path);
    sent->addItem(tmp);
}

// sadly, the ToxFile struct in core only has the file name, not the file path...
// so currently, these don't work as intended (though for now, downloads might work
// whenever they're not saved anywhere custom, thanks to the hack)
// I could do some digging around, but for now I'm tired and others already
// might know it without me needing to dig, so...
void FilesForm::onFileActivated(QListWidgetItem* item) {
    Widget::confirmExecutableOpen(QFileInfo(item->data(Qt::UserRole).toString()));
}

void FilesForm::retranslateUi() {
    headLabel.setText(tr("Transferred Files", "\"Headline\" of the window"));
    main.setTabText(0, tr("Downloads"));
    main.setTabText(1, tr("Uploads"));
}
