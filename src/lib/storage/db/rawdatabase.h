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

#ifndef RAWDATABASE_H
#define RAWDATABASE_H

#include "src/base/strongtype.h"

#include <QByteArray>
#include <QMutex>
#include <QPair>
#include <QQueue>
#include <QRegularExpression>
#include <QString>
#include <QThread>
#include <QVariant>
#include <QVector>

#include <atomic>
#include <cassert>
#include <functional>
#include <memory>

/// The two following defines are required to use SQLCipher
/// They are used by the sqlite3.h header
#define SQLITE_HAS_CODEC
#define SQLITE_TEMP_STORE 2

class sqlite3_stmt;
class sqlite3_context;
class sqlite3_value;
class sqlite3;

using RowId = NamedType<int64_t, struct RowIdTag, Orderable>;
Q_DECLARE_METATYPE(RowId);

namespace lib::db {



class RawDatabase : QObject {
    Q_OBJECT
public:
    class Query {
    public:
        Query(QString query, QVector<QByteArray> blobs = {},
              const std::function<void(RowId)>& insertCallback = {})
                : query{query.toUtf8()}, blobs{blobs}, insertCallback{insertCallback} {}
        Query(QString query, const std::function<void(RowId)>& insertCallback)
                : query{query.toUtf8()}, insertCallback{insertCallback} {}
        Query(QString query, const std::function<void(const QVector<QVariant>&)>& rowCallback)
                : query{query.toUtf8()}, rowCallback{rowCallback} {}
        Query() = default;

    private:
        QByteArray query;
        QVector<QByteArray> blobs;
        std::function<void(RowId)> insertCallback;
        std::function<void(const QVector<QVariant>&)> rowCallback;
        QVector<sqlite3_stmt*> statements;

        friend class RawDatabase;
    };

public:
    RawDatabase(const QString& path, const QString& password, const QByteArray& salt, QObject* parent=nullptr);

    ~RawDatabase();
    bool isOpen();

    bool execNow(const QString& statement);
    bool execNow(const Query& statement);
    bool execNow(const QVector<Query>& statements);

    void execLater(const QString& statement);
    void execLater(const Query& statement);
    void execLater(const QVector<Query>& statements);

    void sync();

public slots:
    bool rename(const QString& newPath);
    bool remove();

protected slots:
    bool open(const QString& path, const QString& hexKey = {});
    void close();
    void process();

private:
    QString anonymizeQuery(const QByteArray& query);
    bool setKey(const QString& hexKey);
    int getUserVersion();
    bool encryptDatabase(const QString& newHexKey);
    bool decryptDatabase();
    bool commitDbSwap(const QString& hexKey);
    bool testUsable();

protected:
    static QString deriveKey(const QString& password, const QByteArray& salt);
    static QString deriveKey(const QString& password);
    static QVariant extractData(sqlite3_stmt* stmt, int col);
    static void regexpInsensitive(sqlite3_context* ctx, int argc, sqlite3_value** argv);
    static void regexpSensitive(sqlite3_context* ctx, int argc, sqlite3_value** argv);

private:
    static void regexp(sqlite3_context* ctx, int argc, sqlite3_value** argv,
                       const QRegularExpression::PatternOptions cs);

    struct Transaction {
        QVector<Query> queries;
        std::atomic_bool* success = nullptr;
        std::atomic_bool* done = nullptr;
    };

private:
    sqlite3* sqlite;
    std::unique_ptr<QThread> workerThread;
    QQueue<Transaction> pendingTransactions;
    QMutex transactionsMutex;
    QString path;
    QByteArray currentSalt;
    QString currentHexKey;
};
}  // namespace lib::db

#endif  // RAWDATABASE_H
