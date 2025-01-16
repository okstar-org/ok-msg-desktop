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

#pragma once

#include <QColor>
#include <QCoreApplication>
#include <QDir>
#include <QPointF>
#include <string>
#include <vector>

namespace Util {
QColor getComplementaryColor(const QColor& clr, const QColor& lineClr = QColor());

bool executeProgram(const QString& path);

inline bool stringTokenizer(const std::string& strSrc, const std::string& strDelimiter,
                            std::vector<std::string>& strList) {
    size_t offsetPrev = 0;
    size_t offsetNext = 0;
    while (true) {
        offsetNext = strSrc.find(strDelimiter, offsetPrev);
        if (std::string::npos == offsetNext) {
            if (offsetPrev != 0) {
                std::string sub = strSrc.substr(offsetPrev);
                strList.push_back(sub);
            }
            break;
        }

        std::string sub = strSrc.substr(offsetPrev, offsetNext - offsetPrev);
        strList.push_back(sub);

        offsetPrev = offsetNext + strDelimiter.length();
    }

    return strList.size() ? true : false;
}

inline bool parseVersionString(const std::string& version, int& major, int& minor, int& revision) {
    std::vector<std::string> strList;
    if (!stringTokenizer(version, ".", strList)) return false;

    if (strList.size() < 3) return false;

    major = atoi(strList[0].c_str());
    minor = atoi(strList[1].c_str());
    revision = atoi(strList[2].c_str());
    return true;
}

inline int compareVersion(const std::string& version1, const std::string& version2) {
    int major1, minor1, rev1;
    int major2, minor2, rev2;
    if (!parseVersionString(version1, major1, minor1, rev1)) return 0;  // exception
    if (!parseVersionString(version2, major2, minor2, rev2)) return 0;  // exception

    if (major1 == major2 && minor1 == minor2 && rev1 == rev2)  // most case
        return 0;

    if (major1 > major2)
        return 1;
    else if (major1 < major2)
        return -1;

    if (minor1 > minor2)
        return 1;
    else if (minor1 < minor2)
        return -1;

    if (rev1 > rev2)
        return 1;
    else if (rev1 < rev2)
        return -1;

    return 0;
}

QPointF calculateNewItemPos(int sceneWidth, int sceneHeight, int mouseX, int mouseY,
                            int targetWidth, int targetHeight, double* lastItemPosX = nullptr,
                            double* lastItemPosY = nullptr);
QPointF calculateNewTextPos(int sceneWidth, int sceneHeight, int mouseX, int mouseY, int textSize,
                            double* lastTextPosX = nullptr, double* lastTextPosY = nullptr);

void HexDump(const void* ptr, int buflen);

inline QString generateFileDownloadPath(const QString* path = 0) {
    QString res;
    if (path)
        res += *path;
    else
        res = QCoreApplication::applicationDirPath();

    res += QDir::separator();
    res += "Download";
    res += QDir::separator();

    QDir dir(res);
    if (!dir.exists()) dir.mkpath(res);
    return res;
}

inline QString toStringFromUtf8(const std::string& str) {
    QString res = QString::fromUtf8(str.c_str());
    return res;
}

inline std::string toUtf8StdString(const QString& str) {
    std::string res;

    QByteArray a = str.toUtf8();

    res.assign(a.data(), a.size());

    return res;
}

inline QString checkAndChangeSameFileName(const QString& path, const QString* baseName = nullptr,
                                          int index = 1) {
    QFileInfo pathInfo(path);
    if (!pathInfo.exists()) return path;

    QString baseNameOnly = baseName ? *baseName : pathInfo.baseName();
    QString newName(baseNameOnly);
    newName += ("(" + QString::number(index) + ")");

    return checkAndChangeSameFileName(
            QDir::toNativeSeparators(pathInfo.path() + QDir::separator() + newName + +"." +
                                     pathInfo.completeSuffix()),
            &baseNameOnly, ++index);
}
};  // namespace Util
