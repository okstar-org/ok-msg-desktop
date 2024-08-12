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

#ifndef FILES_H
#define FILES_H

#include <QBuffer>
#include <QDataStream>
#include <QDebug>
#include <QFileInfo>
#include <QMimeDatabase>
#include "basic_types.h"

namespace ok::base {

enum class FileContentType {
    UNKOWN,
    PDF,
    PNG,
    JPG,
    BMP,
    VIDEO,
    TXT,
    DOC,
    DOCX,
    PPT,
    PPTX,
    XLS,
    XLSX
};

class Files {
public:
    Files();

    static QString GetContentTypeStr(const QString& fileName) {
        QFileInfo info(fileName);
        QMimeDatabase mimeDB;
        return mimeDB.mimeTypeForFile(info).name();
    }

    static FileContentType toContentTypeE(const QString& _contentType) {
        if (_contentType.compare("application/pdf") == 0)  // PDF
        {
            return FileContentType::PDF;
        } else if (_contentType.compare("video/mp4") == 0 ||        // mp4
                   _contentType.compare("video/x-msvideo") == 0 ||  // avi
                   _contentType.compare("video/x-ms-wmv") == 0 ||   // wmv
                   _contentType.compare("video/x-flv") == 0         // flv
        ) {
            return FileContentType::VIDEO;
        } else if (_contentType.compare("application/msword") == 0)  // doc
        {
            return FileContentType::DOC;
        } else if (_contentType.compare("application/"
                                        "vnd.openxmlformats-officedocument."
                                        "wordprocessingml.document") == 0)  // docx
        {
            return FileContentType::DOCX;
        } else if (_contentType.compare("application/vnd.ms-powerpoint") == 0)  // ppt
        {
            return FileContentType::PPT;
        } else if (_contentType.compare("application/"
                                        "vnd.openxmlformats-officedocument."
                                        "presentationml.presentation") == 0)  // pptx
        {
            return FileContentType::PPTX;
        } else if (_contentType.compare("image/png") == 0) {
            return FileContentType::PNG;
        } else if (_contentType.compare("image/jpeg") == 0 ||
                   _contentType.compare("image/pjpeg") == 0) {
            return FileContentType::JPG;
        } else if (_contentType.compare("image/bmp") == 0) {
            return FileContentType::BMP;
        }

        return FileContentType::UNKOWN;
    }

    static FileContentType GetContentType(const QString& filename) {
        QString _contentType = GetContentTypeStr(filename);
        return toContentTypeE(_contentType);
    }

    static bool isImage(const QString& contentType) {
        FileContentType _contentType = toContentTypeE(contentType);
        switch (_contentType) {
            case FileContentType::PNG:
            case FileContentType::BMP:
            case FileContentType::JPG:
                return true;
            default:
                break;
        }

        return false;
    }

    static bool writeTo(const QByteArray& byteArray_, const QString& path) {
        QFile file(path);
        if (!file.open(QIODevice::OpenModeFlag::WriteOnly)) return false;
        file.write(byteArray_);
        file.close();
        return true;
    }

    inline static bool moveFile(const QString& oldPath, QString newPath) {
        QFile file(oldPath);
        if (!file.exists()) return false;
        return file.rename(newPath);
    }

    inline static bool removeFile(const QString& filePath) {
        QFile file(filePath);
        if (file.exists()) {
            return file.remove();
        } else {
            return false;
        }
    }

    inline static QString readStringAll(const QString& path) {
        QFile file(path);
        if (!file.open(QFile::ReadOnly)) {
            return QString{};
        }
        return file.readAll();
    }

    static bool ReadKeyValueLine(const QString& filePath, const QString& delimiter,
                                 Fn<void(QString k, QString v)> fn) {
        QFile file(filePath);

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

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
};

}  // namespace ok::base

#endif  // FILES_H
