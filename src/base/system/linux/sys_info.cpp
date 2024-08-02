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

#include "../../system/sys_info.h"
#include "files.h"

#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QSysInfo>

namespace ok::base {

bool SysInfo::GetCpuInfo(CpuInfo& info) {
    auto arch = QSysInfo::currentCpuArchitecture();
    info.arch = arch == "x86_64" ? "x64" : arch;
    return Files::ReadKeyValueLine("/proc/cpuinfo", ":", [&](const QString& k, const QString& v) {
        if (k == "vendor_id") {
            info.manufacturer = v;
        } else if (k == "model name") {
            info.name = v;
        } else if (k == "cpu cores") {
            info.cores = std::stoi(v.toStdString());
        } else if (k == "processor") {
            info.processors = std::stoi(v.toStdString()) + 1;
        }
    });
}

bool SysInfo::GetOsInfo(OsInfo& info) {
    /**
     * /etc/os-release
     * PRETTY_NAME="Ubuntu 22.04.2 LTS"
     * NAME="Ubuntu"
     * VERSION_ID="22.04"
     */
    info.uniqueId = QSysInfo::machineUniqueId().toBase64();
    info.hostName = QSysInfo::machineHostName();
    info.kernelName = QSysInfo::kernelType();
    info.kernelVersion = QSysInfo::kernelVersion();
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
