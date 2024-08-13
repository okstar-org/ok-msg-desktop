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

#ifndef HISTORY_H
#define HISTORY_H

#include <QDateTime>
#include <QHash>
#include <QPointer>
#include <QVector>

#include <cassert>
#include <cstdint>
#include <memory>
#include <utility>

#include <base/jsons.h>

#include "src/core/FriendId.h"
#include "src/core/toxfile.h"
#include "src/model/message.h"
#include "src/persistence/db/rawdatabase.h"
#include "src/widget/searchtypes.h"

class Profile;
class HistoryKeeper;

enum class HistMessageContentType { message, file };

// struct FileDbInsertionData
//{
//     FileDbInsertionData();
//
//     QString fileId;
//     QString fileName;
//     QString filePath;
//     int64_t size;
//     int direction;
//
//     QString json(){
//         return QString("{\"id:\":\"%1\", \"name\":\"%2\", \"path\":\"%3\", \"size\":%4,
//         \"direction\":%5}")
//                 .arg(fileId).arg(fileName).arg(filePath).arg(size).arg(direction);
//     }
//
//     void parse(const QString &json){
//         auto doc = Jsons::toJSON(json.toUtf8());
//         auto obj = doc.object();
//         fileId = obj.value("id").toString();
//         fileName = obj.value("fileName").toString();
//         filePath = obj.value("path").toString();
//         size = obj.value("size").toInt();
//         direction = obj.value("direction").toInt();
//     }
// };
//
//
// Q_DECLARE_METATYPE(FileDbInsertionData);

enum class MessageState {
    complete,  // 消息发送成功
    pending,   // 消息发送中
    broken,    // 消息失败
    receipt,   // 消息接收
};

class History : public QObject, public std::enable_shared_from_this<History> {
    Q_OBJECT

public:
    struct HistMessage {
        HistMessage() = default;
        HistMessage(RowId id,
                    HistMessageContentType type,
                    MessageState state,
                    QDateTime timestamp,
                    QString sender,
                    QString receiver,
                    QString message,
                    QString dataId)
                : id{id}
                , type{type}
                , state(state)
                , timestamp{std::move(timestamp)}
                , sender{std::move(sender)}
                , receiver{std::move(receiver)}
                , message(std::move(message))
                , dataId{dataId} {
            //            if(type==HistMessageContentType::message){
            //                 data = std::make_shared<QString>(std::move(message));
            //            }else if(type==HistMessageContentType::file){
            //                FileDbInsertionData dbFile;
            //                dbFile.parse(message);
            //                data= std::make_shared<FileDbInsertionData>(std::move(dbFile));
            //            }
        }

        RowId id;
        QDateTime timestamp;
        QString sender;
        QString receiver;
        MessageState state;
        HistMessageContentType type;
        QString message;
        QString dataId;

        [[nodiscard]] QString asMessage() const {
            if (type == HistMessageContentType::message) {
                return message;
            }
            return {};
        }

        [[nodiscard]] FileInfo asFile() const {
            FileInfo file;
            if (type == HistMessageContentType::file) {
                file.parse(message);
            }
            return file;
        }
    };

    struct DateIdx {
        QDate date;
        size_t numMessagesIn;
    };

    explicit History(std::shared_ptr<RawDatabase> db);
    ~History();

    bool isValid();

    bool historyExists(const FriendId& me, const FriendId& friendPk);

    void eraseHistory();
    void removeFriendHistory(const QString& friendPk);

    uint addNewContact(const QString& contactId);

    void addNewMessage(const Message& message,
                       HistMessageContentType type,
                       bool isDelivered,
                       const std::function<void(RowId)>& insertIdCallback = {});

    void addNewFileMessage(const QString& friendPk,
                           const ToxFile& file,
                           const QString& sender,
                           const QDateTime& time,
                           QString const& dispName);

    void setFileMessage(const ToxFile& file);

    QList<HistMessage> getMessageByDataId(const QString& dataId);

    size_t getNumMessagesForFriend(const FriendId& me, const FriendId& friendPk);
    size_t getNumMessagesForFriendBeforeDate(const FriendId& me, const FriendId& friendPk,
                                             const QDateTime& date);

    QList<HistMessage> getMessagesForFriend(const FriendId& me, const FriendId& friendPk,
                                            size_t firstIdx, size_t lastIdx);
    QList<HistMessage> getLastMessageForFriend(const FriendId& me, const FriendId& pk, uint size,
                                               HistMessageContentType type);

    QList<HistMessage> getUndeliveredMessagesForFriend(const FriendId& me,
                                                       const FriendId& friendPk);
    QDateTime getDateWhereFindPhrase(const QString& friendPk,
                                     const QDateTime& from,
                                     QString phrase,
                                     const ParameterSearch& parameter);
    QList<DateIdx> getNumMessagesForFriendBeforeDateBoundaries(const FriendId& friendPk,
                                                               const QDate& from, size_t maxNum);

    void markAsDelivered(RowId messageId);
    void markAsReceipt(RowId messageId);

    void setFriendAlias(const QString& friendPk, const QString& alias);
    QString getFriendAlias(const QString& friendPk);

protected:
    QVector<RawDatabase::Query> generateNewMessageQueries(
            const Message& message,
            HistMessageContentType type,
            bool isDelivered,
            std::function<void(RowId)> insertIdCallback = {});

signals:

    //    void fileInserted(RowId dbId, QString fileId);

private slots:

    void onFileInserted(RowId dbId, QString fileId);

private:
    bool historyAccessBlocked();
    //    static RawDatabase::Query generateFileFinished(RowId fileId,
    //                                                   bool success,
    //                                                   const QString& filePath,
    //                                                   const QByteArray& fileHash);

    std::shared_ptr<RawDatabase> db;

    QHash<QString, int64_t> peers;
    //    struct FileInfo
    //    {
    //        bool finished = false;
    //        bool success = false;
    //        QString filePath;
    //        QByteArray fileHash;
    //        RowId fileId{-1};
    //    };

    // This needs to be a shared pointer to avoid callback lifetime issues
    //    QHash<QString, RowId> fileCached;
    QString makeSqlForFriend(const FriendId& me, const FriendId& friendPk);
    History::HistMessage rowToMessage(const QVector<QVariant>& row);
};

#endif  // HISTORY_H
