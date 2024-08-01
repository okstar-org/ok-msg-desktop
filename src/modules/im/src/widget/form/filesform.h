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

#ifndef FILESFORM_H
#define FILESFORM_H

#include <QLabel>
#include <QListWidgetItem>
#include <QString>
#include <QTabWidget>
#include <QVBoxLayout>

class ContentLayout;
class QListWidget;

class FilesForm : public QObject {
    Q_OBJECT

public:
    FilesForm();
    ~FilesForm();

    bool isShown() const;
    void show(ContentLayout* contentLayout);

public slots:
    void onFileDownloadComplete(const QString& path);
    void onFileUploadComplete(const QString& path);

private slots:
    void onFileActivated(QListWidgetItem* item);

private:
    void retranslateUi();

private:
    QWidget* head;
    QIcon doneIcon;
    QLabel headLabel;
    QVBoxLayout headLayout;
    QTabWidget main;
    QListWidget *sent, *recvd;
};

#endif  // FILESFORM_H
