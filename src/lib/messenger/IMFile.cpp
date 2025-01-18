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

#include "IMFile.h"
#include <bytestream.h>
#include <gloox.h>
#include <inbandbytestream.h>
#include <jinglefiletransfer.h>
#include <jingleibb.h>
#include "IM.h"
#include "IMFile.h"
#include "IMFileTask.h"

namespace lib::messenger {

IMFileSession::IMFileSession(const std::string& sId,
                             gloox::Jingle::Session* session_,
                             const IMPeerId& peerId,
                             IMFile* sender_,
                             File* file)
        : sId(sId), target(peerId), sender(sender_), file(file), session(session_) {

    task = std::make_unique<IMFileTask>(target.toString(), file, sender);
    //    connect(task.get(), &IMFileTask::fileSending,
    //            [&](const std::string& m_friendId, const File& m_file, int m_seq, int m_sentBytes,
    //                bool end) {
    //                for (auto h : sender->getHandlers())
    //                    h->onFileSendInfo(m_friendId, m_file, m_seq, m_sentBytes, end);
    //            });
    //
    //    connect(task.get(), &IMFileTask::fileAbort,
    //            [&](const std::string& m_friendId, const File& m_file, int m_sentBytes) {
    //                for (auto h : sender->getHandlers()) {
    //                    h->onFileSendAbort(m_friendId, m_file, m_sentBytes);
    //                }
    //            });
    //
    //    connect(task.get(), &IMFileTask::fileError,
    //            [&](const std::string& m_friendId, const File& m_file, int m_sentBytes) {
    //                for (auto h : sender->getHandlers()) {
    //                    h->onFileSendError(m_friendId, m_file, m_sentBytes);
    //                }
    //            });
}

void IMFileSession::stop() {
    //    qDebug() << __func__ << "Send file task will be clear." << sId;
    //    if (task->isRunning()) {
    //        task->forceQuit();
    //    }
}

void IMFileSession::start() {
    //    if (task && !task->isRunning()) {
    //        task->start();
    //    }
}

IMFileSession::~IMFileSession() {
    //    qDebug() << __func__ << "sId" << sId;
}

IMFile::IMFile(IM* im) : IMJingle(im) {
    //    qDebug() << __func__;
    im->addIMHandler(this);
}

IMFile::~IMFile() {
    //    qDebug() << __func__;
}


void IMFile::addFileHandler(FileHandler* handler) {
    fileHandlers.push_back(handler);
}

void IMFile::fileRejectRequest(const std::string& friendId, const File& file) {
    auto sId = file.sId;
    //    qDebug() << __func__ << sId;
    rejectFileRequest(friendId, sId);
}

void IMFile::fileAcceptRequest(const std::string& friendId, const File& file) {
    auto sId = file.sId;
    acceptFileRequest(friendId, file);
}

void IMFile::fileCancel(std::string fileId) {
    currentSid.clear();
}

void IMFile::fileFinishRequest(const std::string& friendId, const std::string& sId) {
    finishFileRequest(friendId, sId);
}

void IMFile::fileFinishTransfer(std::string friendId, const std::string& sId) {
    finishFileTransfer(friendId, sId);
}

bool IMFile::fileSendToFriend(const std::string& bare, const File& file) {
    auto resources = im->getOnlineResources(bare);
    if (resources.empty()) {
        //        qWarning() << "Can not find online friends:" << f;
        return false;
    }

    gloox::JID jid(bare);
    for (auto& r : resources) {
        jid.setResource(r);
        sendFileToResource(jid, file);
    }

    return true;
}

void IMFile::addFile(const File& f) {
    m_waitSendFiles.append(f);
}

void IMFile::doStartFileSendTask(const gloox::Jingle::Session* session, const File& file) {
    //    qDebug() << __func__ << file.sId;
    //    m_fileSenderMap.insert(file.id, task);
    //    qDebug() << __func__ << ("Send file task has been stared.") << ((file.id));
}

void IMFile::doStopFileSendTask(const gloox::Jingle::Session* session, const File& file) {
    //    Q_UNUSED(session)
    //    qDebug() << __func__ << file.sId;
    //    auto* task = m_fileSenderMap.value(file.sId);
    //    if (!task) {
    //        return;
    //    }
    //
    //    qDebug() << __func__ << "Send file task will be clear." << file.id;
    //    if (task->isRunning()) {
    //        task->forceQuit();
    //    }
    //    disconnect(task);
    //    delete task;

    // 返回截断后续处理
    //    m_fileSenderMap.remove(file.sId);
    //    qDebug() << "Send file task has been clean." << file.id;
}

void IMFile::clearSessionInfo(const std::string& sId) {
    auto sess = m_fileSessionMap.find(sId);
    if (sess == m_fileSessionMap.end()) {
        return;
    }
    m_fileSessionMap.erase(sId);
}

/**
 * 文件传输
 * @param friendId
 * @param file
 */
void IMFile::rejectFileRequest(const std::string& friendId, const std::string& sId) {
    //    qDebug() << __func__ << friendId << sId;
    IMFileSession* fs = findSession(sId);
    if (fs) {
        auto s = fs->getJingleSession();
        s->sessionTerminate(
                new gloox::Jingle::Session::Reason(gloox::Jingle::Session::Reason::Decline));
        clearSessionInfo(sId);
    }
}

void IMFile::acceptFileRequest(const std::string& friendId, const File& file) {
    //    qDebug() << __func__ << "file:" << file.name << "sId:" << file.sId;
    auto it = m_fileSessionMap.find(file.sId);
    if (it == m_fileSessionMap.end()) {
        //        qWarning() << "Unable to find session sId:" << file.sId;
        return;
    }
    auto session = it->second;
    auto pSession = session->getJingleSession();
    if (pSession) {
        // 协议：https://xmpp.org/extensions/xep-0234.html#requesting
        gloox::Jingle::PluginList pluginList;
        gloox::Jingle::FileTransfer::FileList files;
        files.push_back(
                gloox::Jingle::FileTransfer::File{.name = (file.name), .size = (long)file.size});
        auto ftf = new gloox::Jingle::FileTransfer(gloox::Jingle::FileTransfer::Received, files);
        auto ibb = new gloox::Jingle::IBB((file.txIbb.sid), file.txIbb.blockSize);
        pluginList.emplace_back(ftf);
        pluginList.emplace_back(ibb);

        auto c = new gloox::Jingle::Content("file", pluginList);
        pSession->sessionAccept(c);
    }
}

void IMFile::finishFileRequest(const std::string& friendId, const std::string& sId) {
    //    qDebug() << __func__ << "sId:" << (sId);
    auto* s = findSession(sId);
    if (!s) {
        //        qWarning() << "Can not find file session" << sId;
        return;
    }
    s->getJingleSession()->sessionTerminate(
            new gloox::Jingle::Session::Reason(gloox::Jingle::Session::Reason::Success));
}

void IMFile::finishFileTransfer(const std::string& friendId, const std::string& sId) {
    //    qDebug() << __func__ << "sId:" << (sId);
    finishFileRequest(friendId, sId);
}

bool IMFile::sendFile(const std::string& bare, const File& file) {
    //    qDebug() << __func__ << friendId << (file.name);
    if (file.id.empty()) {
        //        qWarning() << "file id is no existing";
        return false;
    }

    //    auto bare = stdstring(friendId);
    auto resources = im->getOnlineResources(bare);
    if (resources.empty()) {
        //        qWarning() << "目标用户不在线！";
        return false;
    }

    gloox::JID jid(bare);
    for (auto& r : resources) {
        jid.setResource(r);
        sendFileToResource(jid, file);
    }

    return true;
}

bool IMFile::sendFileToResource(const gloox::JID& jid, const File& file) {
    //    qDebug() << __func__ << qstring(jid.full()) << "file:" << file.name;
    if (file.id.empty()) {
        //        qWarning() << "file's id can not be empty!";
        return false;
    }
    if (file.sId.empty()) {
        //        qWarning() << "file's sid can not be empty!";
        return false;
    }

    auto session = getIM()->createSession(jid, (file.sId), this);
    if (!session) {
        //        qDebug() << "Can not create session!";
        return false;
    }

    gloox::Jingle::PluginList pl;

    // offer-file
    gloox::Jingle::FileTransfer::FileList files;
    gloox::Jingle::FileTransfer::File f = {.name = (file.name),
                                           .size = static_cast<long>(file.size)};

    files.emplace_back(f);
    auto ft = new gloox::Jingle::FileTransfer(gloox::Jingle::FileTransfer::Offer, files);
    pl.emplace_back(ft);

    // ibb
    auto ibb = new gloox::Jingle::IBB((file.id), 4096);
    pl.emplace_back(ibb);

    // content
    auto jc = new gloox::Jingle::Content("offer-a-file", pl);
    session->sessionInitiate(jc);

    // 缓存文件
    auto& nf = const_cast<File&>(file);
    nf.sId = (session->sid());
    addFile(nf);

    currentSid = file.sId;

    return true;
}

bool IMFile::doSessionAccept(gloox::Jingle::Session* session,
                             const gloox::Jingle::Session::Jingle* jingle,
                             const lib::messenger::IMPeerId& peerId) {
    SESSION_CHECK(currentSid);

    auto sId = (session->sid());
    //    qDebug() << __func__ << "sId:" << sId;

    ortc::OJingleContentMap content;
    content.sdpType = ortc::JingleSdpType::Answer;
    ParseAV(jingle, content);

    if (!content.isValid()) {
        //        qWarning() << "Is no file session";
        return false;
    }

    //    for (auto h : fileHandlers) {
    //        for (const auto& item : content.getContents()) {
    //            auto& f = item.second;
    //            if (!f.isFile()) return false;
    //
    //            qDebug() << "file id:" << qstring(f.ibb.sId) << "name:" << qstring(f.name);
    //            File file{.id = qstring(f.ibb.sId),
    //                      .sId = sId,
    //                      .name = qstring(f.file.name),
    //                      .status = FileStatus::TRANSMITTING,
    //                      .direction = FileDirection::RECEIVING};
    //            h->onFileRequest(peerId.toFriendId(), file);
    //        }
    //    }

    auto it = m_fileSessionMap.find(sId);
    if (it == m_fileSessionMap.end()) {
        //        qWarning() << "File session is existing, the sId is" << sId;
        return false;
    }

    // 创建session
    for (auto& file : m_waitSendFiles) {
        auto s = new IMFileSession(sId, session, peerId, this, &file);
        s->start();
        m_fileSessionMap.insert(std::make_pair(sId, s));
    }

    return true;
}

bool IMFile::doSessionInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMFile::doContentAdd(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMFile::doContentRemove(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMFile::doContentModify(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMFile::doContentAccept(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMFile::doContentReject(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMFile::doTransportInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMFile::doTransportAccept(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMFile::doTransportReject(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMFile::doTransportReplace(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMFile::doSecurityInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMFile::doDescriptionInfo(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMFile::doSourceAdd(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    SESSION_CHECK(currentSid);
    return true;
}

bool IMFile::doInvalidAction(const gloox::Jingle::Session::Jingle*, const IMPeerId&) {
    if (currentSid.empty()) {
        return false;
    }
    return true;
}

void IMFile::onConnecting()
{

}

void IMFile::onConnected()
{

}

void IMFile::onDisconnected(int)
{

}

void IMFile::onStarted()
{
    auto client = im->getClient();
    assert(client);
    client->registerIqHandler(this, gloox::ExtIBB);
    // session manager
    im->addSessionHandler(this);
}

void IMFile::onStopped()
{

}

bool IMFile::doSessionInitiate(gloox::Jingle::Session* session,
                               const gloox::Jingle::Session::Jingle* jingle,
                               const IMPeerId& peerId) {
    auto sId = (session->sid());
    //    qDebug() << __func__ << "sId:" << sId;

    ortc::OJingleContentMap contentAv;
    contentAv.sdpType = ortc::JingleSdpType::Offer;
    ParseAV(jingle, contentAv);

    for (auto& item : contentAv.getContents()) {
        if (!item.second.isFile()) {
            return false;
        }
        auto& f = item.second;
        File* file0 = new File{.id = (f.ibb.sId),
                               .sId = sId,
                               .name = (f.file.name),
                               .path = {},
                               .size = static_cast<uint32_t>(f.file.size),
                               .status = FileStatus::INITIALIZING,
                               .direction = FileDirection::RECEIVING};
        for (auto h : fileHandlers) {
            h->onFileRequest(peerId.toFriendId(), *file0);
        }
        // 创建session
        auto s = new IMFileSession(sId, session, peerId, this, file0);
        m_fileSessionMap.insert(std::make_pair(sId, s));
    }
    currentSid = sId;
    return true;
}

bool IMFile::handleIq(const gloox::IQ& iq) {
    const auto* ibb = iq.findExtension<gloox::InBandBytestream::IBB>(gloox::ExtIBB);
    if (!ibb) {
        return false;
    }

    IMContactId friendId((iq.from().bare()));
    //    qDebug() << __func__ << std::string("IBB stream id:%1").arg((ibb->sid()));

    switch (ibb->type()) {
        case gloox::InBandBytestream::IBBOpen: {
            //            qDebug() << __func__ << std::string("Open");
            break;
        }
        case gloox::InBandBytestream::IBBData: {
            auto sId = (ibb->sid());

            //            qDebug() << __func__ << std::string("Data seq:%1").arg(ibb->seq());
            for (auto h : fileHandlers) {
                h->onFileRecvChunk(friendId.toString(), sId, ibb->seq(), ibb->data());
            }

            break;
        }
        case gloox::InBandBytestream::IBBClose: {
            //            qDebug() << __func__ << std::string("Close");
            auto fileId = (ibb->sid());
            for (auto& p : m_fileSessionMap) {
                auto k = p.first;
                auto ss = p.second;
                auto file = ss->getFile();
                if (file->id == fileId) {
                    ss->getJingleSession()->sessionTerminate(new gloox::Jingle::Session::Reason(
                            gloox::Jingle::Session::Reason::Success));
                    m_fileSessionMap.erase(k);
                    delete ss;
                    break;
                }
            }

            for (auto h : fileHandlers) {
                h->onFileRecvFinished(friendId.toString(), fileId);
            }

            break;
        }
        default: {
        }
    }

    gloox::IQ riq(gloox::IQ::IqType::Result, iq.from(), iq.id());
    im->getClient()->send(riq);
    return true;
}

bool IMFile::doSessionTerminate(gloox::Jingle::Session* session,
                                const gloox::Jingle::Session::Jingle*,
                                const IMPeerId&) {
    auto sId = (session->sid());

    if (currentSid.empty()) {
        return false;
    }
    clearSessionInfo(sId);
    currentSid.clear();
    return true;
}

}  // namespace lib::messenger
