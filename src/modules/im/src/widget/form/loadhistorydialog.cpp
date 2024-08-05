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

#include "loadhistorydialog.h"
#include <QCalendarWidget>
#include <QDate>
#include <QTextCharFormat>
#include "src/model/ichatlog.h"
#include "src/nexus.h"
#include "src/persistence/history.h"
#include "src/persistence/profile.h"
#include "ui_loadhistorydialog.h"

LoadHistoryDialog::LoadHistoryDialog(const IChatLog* chatLog, QWidget* parent)
        : QDialog(parent), ui(new Ui::LoadHistoryDialog), chatLog(chatLog) {
    ui->setupUi(this);
    highlightDates(QDate::currentDate().year(), QDate::currentDate().month());
    connect(ui->fromDate, &QCalendarWidget::currentPageChanged, this,
            &LoadHistoryDialog::highlightDates);
}

LoadHistoryDialog::LoadHistoryDialog(QWidget* parent)
        : QDialog(parent), ui(new Ui::LoadHistoryDialog) {
    ui->setupUi(this);
}

LoadHistoryDialog::~LoadHistoryDialog() { delete ui; }

QDateTime LoadHistoryDialog::getFromDate() {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 15, 0))
    QDateTime res(ui->fromDate->selectedDate().startOfDay());
#else
    QDateTime res(ui->fromDate->selectedDate());
#endif
    if (res.date().month() != ui->fromDate->monthShown() ||
        res.date().year() != ui->fromDate->yearShown()) {
        QDate newDate(ui->fromDate->yearShown(), ui->fromDate->monthShown(), 1);
        res.setDate(newDate);
    }

    return res;
}

void LoadHistoryDialog::setTitle(const QString& title) { setWindowTitle(title); }

void LoadHistoryDialog::setInfoLabel(const QString& info) { ui->fromLabel->setText(info); }

void LoadHistoryDialog::highlightDates(int year, int month) {
    History* history = Nexus::getProfile()->getHistory();
    QDate monthStart(year, month, 1);
    QDate monthEnd(year, month + 1, 1);

    // Max 31 days in a month
    auto dateIdxs = chatLog->getDateIdxs(monthStart, 31);

    QTextCharFormat format;
    format.setFontWeight(QFont::Bold);

    QCalendarWidget* calendar = ui->fromDate;
    for (const auto& item : dateIdxs) {
        if (item.date < monthEnd) {
            calendar->setDateTextFormat(item.date, format);
        }
    }
}
