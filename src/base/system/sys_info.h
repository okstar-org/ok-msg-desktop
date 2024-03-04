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

#ifndef BASE_SYSTEM_SYS_INFO_H_
#define BASE_SYSTEM_SYS_INFO_H_

#include "base/basic_types.h"
#include <QString>
#include <QFile>
#include <QTextStream>
#include <map>
#include <string>

namespace base {

static bool ReadLineValue(const QString &filePath, const QString &delimiter,
                          Fn<void(QString k, QString v)> fn) {

  QFile file(filePath);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return false;

  QTextStream stream(&file);
  QString line;
  while (stream.readLineInto(&line)) {
    auto arr = line.split(delimiter);
    if (arr.size() == 2 && !arr[1].isEmpty()) {
      fn(arr[0].trimmed(), arr[1].trimmed());
    }
  }
  file.close();
  return true;
}


typedef enum {
  X86,
  X64
} Arch;

typedef struct {
  QString manufacturer;
  QString name;
  qint32 cores;
  qint32 processors;
  QString arch;
} CpuInfo;

typedef struct {
  QString name;
  QString version;
  QString prettyName;
  QString kernelName;
  QString kernelVersion;
  QString hostName;
  QString uniqueId;
} OsInfo;

class SysInfo {
public:
  // Returns the host operating system information.
  static bool GetOsInfo(OsInfo &info);

  // Returns the host cpu information.
  static bool GetCpuInfo(CpuInfo &info);
};

class SystemInfo {
public:
  static SystemInfo* instance();
  QString osVersion();
  OsInfo osInfo();
  CpuInfo cpuInfo();
};

} // namespace base

#endif // BASE_SYSTEM_SYS_INFO_H_
