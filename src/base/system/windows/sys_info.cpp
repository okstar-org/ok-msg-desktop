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

#include "base/system/sys_info.h"
#include <QCoreApplication>
#include <QDir>
#include <QProcess>
#include <QStandardPaths>
#include <algorithm>

namespace ok::base {

bool SysInfo::GetCpuInfo(CpuInfo& info) {
    auto arch = QSysInfo::currentCpuArchitecture();
    info.arch = arch == "x86_64" ? "x64" : arch;
    // 获取CPU型号和核心数
    return true;
}

bool SysInfo::GetOsInfo(OsInfo& info) {
    info.uniqueId = QSysInfo::machineUniqueId().toBase64();
    info.hostName = QSysInfo::machineHostName();
    info.kernelName = QSysInfo::kernelType();
    info.prettyName = QSysInfo::prettyProductName();
    info.name = QSysInfo::productType();
    info.version = QSysInfo::productVersion();
    return true;
}

SystemInfo* SystemInfo::instance() {
    static SystemInfo* instance_ = nullptr;
    if (!instance_) instance_ = new SystemInfo();
    return instance_;
}

QString SystemInfo::osVersion() { return osInfo().name; }

OsInfo SystemInfo::osInfo() {
    OsInfo info;
    SysInfo::GetOsInfo(info);
    return info;
}

CpuInfo SystemInfo::cpuInfo() {
    CpuInfo info;
    SysInfo::GetCpuInfo(info);
    return info;
}

}  // namespace ok::base
