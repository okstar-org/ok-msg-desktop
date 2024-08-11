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

#ifndef SETTINGSSERIALIZER_H
#define SETTINGSSERIALIZER_H

#include "src/core/toxencrypt.h"

#include <QDataStream>
#include <QSettings>
#include <QString>
#include <QVector>

class SettingsSerializer {
public:
    SettingsSerializer(QString filePath, const ToxEncrypt* passKey = nullptr);

    static bool isSerializedFormat(QString filePath);

    void load();
    void save();

    void beginGroup(const QString& prefix);
    void endGroup();

    int beginReadArray(const QString& prefix);
    void beginWriteArray(const QString& prefix, int size = -1);
    void endArray();
    void setArrayIndex(int i);

    void setValue(const QString& key, const QVariant& value);
    QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;

private:
    enum class RecordTag : uint8_t {
        Value = 0,
        GroupStart = 1,
        ArrayStart = 2,
        ArrayValue = 3,
        ArrayEnd = 4,
    };
    friend QDataStream& writeStream(QDataStream& dataStream,
                                    const SettingsSerializer::RecordTag& tag);
    friend QDataStream& readStream(QDataStream& dataStream, SettingsSerializer::RecordTag& tag);

    struct Value {
        Value() : group{-2}, array{-2}, arrayIndex{-2}, key{QString()}, value{} {}
        Value(qint64 group, qint64 array, int arrayIndex, QString key, QVariant value)
                : group{group}, array{array}, arrayIndex{arrayIndex}, key{key}, value{value} {}
        qint64 group;
        qint64 array;
        int arrayIndex;
        QString key;
        QVariant value;
    };

    struct Array {
        qint64 group;
        int size;
        QString name;
        QVector<int> values;
    };

private:
    const Value* findValue(const QString& key) const;
    Value* findValue(const QString& key);
    void readSerialized();
    void readIni();
    void removeValue(const QString& key);
    void removeGroup(int group);
    void writePackedVariant(QDataStream& dataStream, const QVariant& v);

private:
    QString path;
    const ToxEncrypt* passKey;
    int group, array, arrayIndex;
    QStringList groups;
    QVector<Array> arrays;
    QVector<Value> values;
    static const char magic[];
};

#endif  // SETTINGSSERIALIZER_H
