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

#include <windows.h>
#include <QApplication>
#include <string>
#include "base/autorun.h"

#ifdef UNICODE

using tstring = std::wstring;
static inline tstring toTString(QString s) { return s.toStdWString(); }
#else
using tstring = std::string;
static inline tstring toTString(QString s) { return s.toStdString(); }
#endif

namespace Platform {
inline tstring currentCommandLine() {
    return toTString("\"" + QApplication::applicationFilePath().replace('/', '\\'));
}

inline tstring currentRegistryKeyName() { return toTString(APPLICATION_EXE_NAME " - "); }

bool setAutorun(bool on) {
    HKEY key = 0;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
                     0, KEY_ALL_ACCESS, &key) != ERROR_SUCCESS)
        return false;

    bool result = false;
    tstring keyName = currentRegistryKeyName();

    if (on) {
        tstring path = currentCommandLine();
        result = RegSetValueEx(key, keyName.c_str(), 0, REG_SZ, (PBYTE)path.c_str(),
                               path.length() * sizeof(TCHAR)) == ERROR_SUCCESS;
    } else
        result = RegDeleteValue(key, keyName.c_str()) == ERROR_SUCCESS;

    RegCloseKey(key);
    return result;
}

bool getAutorun() {
    HKEY key = 0;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
                     0, KEY_ALL_ACCESS, &key) != ERROR_SUCCESS)
        return false;

    tstring keyName = currentRegistryKeyName();

    TCHAR path[MAX_PATH] = {0};
    DWORD length = sizeof(path);
    DWORD type = REG_SZ;
    bool result = false;

    if (RegQueryValueEx(key, keyName.c_str(), 0, &type, (PBYTE)path, &length) == ERROR_SUCCESS &&
        type == REG_SZ)
        result = true;

    RegCloseKey(key);
    return result;
}

}  // namespace Platform