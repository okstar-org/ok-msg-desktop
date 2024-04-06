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
#ifndef TIMES_H
#define TIMES_H

#include <QDateTime>
#include <sstream>
#include <string>

namespace base {

class Times {
public:
  static QDateTime now(){
    return QDateTime::currentDateTime();
  }

  static std::string formatTime(int ms) {
    int s = 1000;
    int mi = s * 60;
    int hh = mi * 60;
    int dd = hh * 24;

    long day = ms / dd;
    long hour = (ms - day * dd) / hh;
    long minute = (ms - day * dd - hour * hh) / mi;
    long second = (ms - day * dd - hour * hh - minute * mi) / s;
    //        long milliSecond = ms - day * dd - hour * hh - minute * mi -
    //        second * s;

    //        std::to_string(day)

    //        QString hou = QString::number(hour,10);
    //        QString min = QString::number(minute,10);
    //        QString sec = QString::number(second,10);
    //        QString msec = QString::number(milliSecond,10);

    // qDebug() << "minute:" << min << "second" << sec << "ms" << msec <<endl;
    std::stringstream ss;
    ss << std::to_string(hour) << ":" << std::to_string(minute) << ":"
       << std::to_string(second);
    return ss.str();
  }
};

} // namespace base

#endif // TIMES_H
