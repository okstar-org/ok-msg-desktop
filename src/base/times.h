/*
 * Copyright (c) 2022 船山信息 chuanshaninfo.com
 * The project is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PubL v2 for more details.
 */
#ifndef OK_BASE_TIMES_H
#define OK_BASE_TIMES_H

#include <QDateTime>
#include <QMap>
#include <QString>
namespace ok::base {

inline QString secondsToDHMS(quint32 duration) {
    QString res;
    QString cD = "";  //
    quint32 seconds = duration % 60;
    duration /= 60;
    quint32 minutes = duration % 60;
    duration /= 60;
    quint32 hours = duration % 24;
    quint32 days = duration / 24;

    // I assume no one will ever have call longer than a month
    if (days) {
        return cD + res.asprintf("%dd%02dh %02dm %02ds", days, hours, minutes, seconds);
    }

    if (hours) {
        return cD + res.asprintf("%02dh %02dm %02ds", hours, minutes, seconds);
    }

    if (minutes) {
        return cD + res.asprintf("%02dm %02ds", minutes, seconds);
    }

    return cD + res.asprintf("%02ds", seconds);
}

enum class ReadableTime {
    Today,
    Yesterday,
    ThisWeek,
    ThisMonth,
    Month1Ago,
    Month2Ago,
    Month3Ago,
    Month4Ago,
    Month5Ago,
    LongAgo,
    Never
};

static const int LAST_TIME = static_cast<int>(ReadableTime::Never);

inline ReadableTime getTimeBucket(const QDateTime& date) {
    if (date == QDateTime()) {
        return ReadableTime::Never;
    }

    QDate today = QDate::currentDate();
    // clang-format off
  const QMap<ReadableTime, QDate> dates {
        { ReadableTime::Today,     today.addDays(0)    },
        { ReadableTime::Yesterday, today.addDays(-1)   },
        { ReadableTime::ThisWeek,  today.addDays(-6)   },
        { ReadableTime::ThisMonth, today.addMonths(-1) },
        { ReadableTime::Month1Ago, today.addMonths(-2) },
        { ReadableTime::Month2Ago, today.addMonths(-3) },
        { ReadableTime::Month3Ago, today.addMonths(-4) },
        { ReadableTime::Month4Ago, today.addMonths(-5) },
        { ReadableTime::Month5Ago, today.addMonths(-6) },
    };
    // clang-format on

    for (ReadableTime time : dates.keys()) {
        if (dates[time] <= date.date()) {
            return time;
        }
    }

    return ReadableTime::LongAgo;
}

class Times {
public:
    inline static QDateTime now() { return QDateTime::currentDateTime(); }

    inline static QString formatTime(const QDateTime& dateTime, const QString& fmt) {
        if (dateTime.isNull()) {
            return {};
        }

        if (fmt.isNull() || fmt.isEmpty()) {
            return dateTime.toString();
        }

        return dateTime.toString(fmt);
    }

    inline static qint64 timeUntilTomorrow() {
        QDateTime now = QDateTime::currentDateTime();
        QDateTime tomorrow = now.addDays(1);  // Tomorrow.
        tomorrow.setTime(QTime());            // Midnight.
        return now.msecsTo(tomorrow);
    }
};

}  // namespace ok::base

#endif  // TIMES_H
