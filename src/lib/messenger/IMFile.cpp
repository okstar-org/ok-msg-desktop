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

QString File::toString() const {
    return QString("{id:%1, sId:%2, name:%3, path:%4, size:%5, status:%6, direction:%7}")
            .arg(id)
            .arg(sId)
            .arg(name)
            .arg(path)
            .arg(size)
            .arg((int)status)
            .arg((int)direction);
}

QDebug& operator<<(QDebug& debug, const File& f) {
    QDebugStateSaver saver(debug);
    debug.nospace() << f.toString();
    return debug;
}

IMFileSession::IMFileSession(const QString& sId,
                             gloox::Jingle::Session* session_,
                             const IMPeerId& peerId,
                             IMFile* sender_,
                             File* file)
        : sId(sId), target(peerId), sender(sender_), file(file), session(session_) {
    qDebug() << __func__ << "Create file task:" << sId;

    task = std::make_unique<IMFileTask>(target.toString(), file, sender);
    connect(task.get(), &IMFileTask::fileSending,
            [&](const QString& m_friendId, const File& m_file, int m_seq, int m_sentBytes,
                bool end) {
                for (auto h : sender->getHandlers())
                    h->onFileSendInfo(m_friendId, m_file, m_seq, m_sentBytes, end);
            });

    connect(task.get(), &IMFileTask::fileAbort,
            [&](const QString& m_friendId, const File& m_file, int m_sentBytes) {
                for (auto h : sender->getHandlers()) {
                    h->onFileSendAbort(m_friendId, m_file, m_sentBytes);
                }
            });

    connect(task.get(), &IMFileTask::fileError,
            [&](const QString& m_friendId, const File& m_file, int m_sentBytes) {
                for (auto h : sender->getHandlers()) {
                    h->onFileSendError(m_friendId, m_file, m_sentBytes);
                }
            });
}

void IMFileSession::stop() {
    qDebug() << __func__ << "Send file task will be clear." << sId;
    if (task->isRunning()) {
        task->forceQuit();
    }
}

void IMFileSession::start() {
    if (task && !task->isRunning()) {
        task->start();
    }
}

IMFileSession::~IMFileSession() {
    qDebug() << __func__ << "sId" << sId;
}

IMFile::IMFile(IM* im, QObject* parent) : IMJingle(im, parent) {
    qRegisterMetaType<File>("File");

    connect(im, &IM::started, this, &IMFile::onImStartedFile);
}

IMFile::~IMFile() {
    qDebug() << __func__;
}

void IMFile::parse(const gloox::Jingle::Session::Jingle* jingle,
                   ortc::OJingleContentFile& content) {
    const auto& sId = (jingle->sid());
    const gloox::Jingle::Plugin* c = nullptr;
    for (auto p : jingle->plugins()) {
        auto pt = p->pluginType();
        switch (pt) {
            case gloox::Jingle::JinglePluginType::PluginContent: {
                c = p;
                break;
            }
            default:
                break;
        }
    }

    if (!c) {
        return;
    }

    auto file = c->findPlugin<gloox::Jingle::FileTransfer>(gloox::Jingle::PluginFileTransfer);
    auto ibb = c->findPlugin<gloox::Jingle::IBB>(gloox::Jingle::PluginIBB);
    if (file && ibb) {
        for (auto& f : file->files()) {
            auto id = (ibb->sid());
            qDebug() << "file:" << qstring(id) << "sId:" << qstring(sId);
            ortc::OFile file0 = {.id = id,
                                 .sId = sId,
                                 .date = f.date,
                                 .hash = f.hash,
                                 .hash_algo = f.hash_algo,
                                 .size = f.size,
                                 .range = f.range,
                                 .offset = f.offset};
            file0.name = f.name;
            content.contents.push_back(file0);
        }
    }
}

void IMFile::onImStartedFile() {
    auto client = im->getClient();
    assert(client);
    client->registerIqHandler(this, gloox::ExtIBB);

    // jingle file
    auto disco = client->disco();
    disco->addFeature(gloox::XMLNS_JINGLE_FILE_TRANSFER);
    disco->addFeature(gloox::XMLNS_JINGLE_FILE_TRANSFER4);
    disco->addFeature(gloox::XMLNS_JINGLE_FILE_TRANSFER5);
    disco->addFeature(gloox::XMLNS_JINGLE_FILE_TRANSFER_MULTI);
    disco->addFeature(gloox::XMLNS_JINGLE_IBB);

    // session manager
    im->sessionManager()->registerPlugin(new gloox::Jingle::Content());
    im->sessionManager()->registerPlugin(new gloox::Jingle::FileTransfer());
    im->sessionManager()->registerPlugin(new gloox::Jingle::IBB());
    im->addSessionHandler(this);
}

void IMFile::addFileHandler(FileHandler* handler) {
    fileHandlers.push_back(handler);
}

void IMFile::fileRejectRequest(const QString& friendId, const File& file) {
    auto sId = file.sId;
    qDebug() << __func__ << sId;
    rejectFileRequest(friendId, sId);
}

void IMFile::fileAcceptRequest(const QString& friendId, const File& file) {
    auto sId = file.sId;
    qDebug() << __func__ << "sId:" << sId;
    acceptFileRequest(friendId, file);
}

void IMFile::fileCancel(QString fileId) {
    qDebug() << __func__ << "file" << fileId;
    currentSid.clear();
}

void IMFile::fileFinishRequest(QString friendId, const QString& sId) {
    qDebug() << __func__ << sId;
    finishFileRequest(friendId, sId);
}

void IMFile::fileFinishTransfer(QString friendId, const QString& sId) {
    qDebug() << __func__ << sId;
    finishFileTransfer(friendId, sId);
}

bool IMFile::fileSendToFriend(const QString& f, const File& file) {
    qDebug() << __func__ << "friend:" << f << "file:" << file.id;

    auto bare = stdstring(f);

    auto resources = im->getOnlineResources(bare);
    if (resources.empty()) {
        qWarning() << "Can not find online friends:" << f;
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
    Q_UNUSED(session)
    qDebug() << __func__ << file.sId;
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

void IMFile::clearSessionInfo(const QString& sId) {
    IMFileSession* sess = m_fileSessionMap.value(sId);
    if (!sess) {
        qWarning() << __func__ << "Session is no existing, sId:" << sId;
        return;
    }
    m_fileSessionMap.remove(sId);
}

/**
 * 文件传输
 * @param friendId
 * @param file
 */
void IMFile::rejectFileRequest(const QString& friendId, const QString& sId) {
    qDebug() << __func__ << friendId << sId;
    IMFileSession* fs = findSession(sId);
    if (fs) {
        auto s = fs->getJingleSession();
        s->sessionTerminate(
                new gloox::Jingle::Session::Reason(gloox::Jingle::Session::Reason::Decline));
        clearSessionInfo(sId);
    }
}

void IMFile::acceptFileRequest(const QString& friendId, const File& file) {
    qDebug() << __func__ << "file:" << file.name << "sId:" << file.sId;
    auto session = m_fileSessionMap.value(file.sId);
    if (!session) {
        qWarning() << "Unable to find session sId:" << file.sId;
        return;
    }

    // 协议：https://xmpp.org/extensions/xep-0234.html#requesting
    gloox::Jingle::PluginList pluginList;

    gloox::Jingle::FileTransfer::FileList files;
    files.push_back(gloox::Jingle::FileTransfer::File{.name = stdstring(file.name),
                                                      .size = (long)file.size});

    auto ftf = new gloox::Jingle::FileTransfer(gloox::Jingle::FileTransfer::Request, files);

    auto ibb = new gloox::Jingle::IBB(stdstring(file.txIbb.sid), file.txIbb.blockSize);

    pluginList.emplace_back(ftf);
    pluginList.emplace_back(ibb);

    auto c = new gloox::Jingle::Content("file", pluginList);
    auto pSession = session->getJingleSession();
    if (pSession) pSession->sessionAccept(c);
}

void IMFile::finishFileRequest(const QString& friendId, const QString& sId) {
    qDebug() << __func__ << "sId:" << (sId);
    auto* s = findSession(sId);
    if (!s) {
        qWarning() << "Can not find file session" << sId;
        return;
    }
    s->getJingleSession()->sessionTerminate(
            new gloox::Jingle::Session::Reason(gloox::Jingle::Session::Reason::Success));
}

void IMFile::finishFileTransfer(const QString& friendId, const QString& sId) {
    qDebug() << __func__ << "sId:" << (sId);
    finishFileRequest(friendId, sId);
}

bool IMFile::sendFile(const QString& friendId, const File& file) {
    qDebug() << __func__ << friendId << (file.name);
    if (file.id.isEmpty()) {
        qWarning() << "file id is no existing";
        return false;
    }

    auto bare = stdstring(friendId);
    auto resources = im->getOnlineResources(bare);
    if (resources.empty()) {
        qWarning() << "目标用户不在线！";
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
    qDebug() << __func__ << qstring(jid.full()) << "sId:" << file.sId;
    auto session = getIM()->createSession(jid, stdstring(file.sId), this);
    if (!session) {
        qDebug() << "Can not create session!";
        return false;
    }

    gloox::Jingle::PluginList pl;

    // offer-file
    gloox::Jingle::FileTransfer::FileList files;
    gloox::Jingle::FileTransfer::File f;
    f.name = stdstring(file.name);
    f.size = file.size;
    files.emplace_back(f);
    auto ft = new gloox::Jingle::FileTransfer(gloox::Jingle::FileTransfer::Offer, files);
    pl.emplace_back(ft);

    // ibb
    auto ibb = new gloox::Jingle::IBB(stdstring(file.id), 4096);
    pl.emplace_back(ibb);

    // content
    auto jc = new gloox::Jingle::Content("offer-a-file", pl);
    session->sessionInitiate(jc);

    // 缓存文件
    auto& nf = const_cast<File&>(file);
    nf.sId = qstring(session->sid());
    addFile(nf);

    return true;
}

bool IMFile::doSessionAccept(gloox::Jingle::Session* session,
                             const gloox::Jingle::Session::Jingle* jingle,
                             const lib::messenger::IMPeerId& peerId) {
    SESSION_CHECK(currentSid);

    auto sId = qstring(session->sid());
    qDebug() << __func__ << "sId:" << sId;

    ortc::OJingleContentFile cfile;
    cfile.sdpType = ortc::JingleSdpType::Answer;
    parse(jingle, cfile);

    if (!cfile.isValid()) {
        qWarning() << "Is no file session";
        return false;
    }

    for (auto h : fileHandlers) {
        auto& f = cfile.contents.at(0);
        qDebug() << "file id:" << qstring(f.id) << "name:" << qstring(f.name);
        File file{.id = qstring(f.id),
                  .sId = sId,
                  .name = qstring(f.name),
                  .status = FileStatus::TRANSMITTING,
                  .direction = FileDirection::RECEIVING};
        h->onFileRequest(peerId.toFriendId(), file);
    }

    IMFileSession* sess = m_fileSessionMap.value(sId);
    if (sess) {
        qWarning() << "File session is existing, the sId is" << sId;
        return false;
    }

    // 创建session
    for (auto& file : m_waitSendFiles) {
        auto s = new IMFileSession(sId, session, peerId, this, &file);
        s->start();
        m_fileSessionMap.insert(sId, s);
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
    if (currentSid.isEmpty()) {
        return false;
    }
    return true;
}

bool IMFile::doSessionInitiate(gloox::Jingle::Session* session,
                               const gloox::Jingle::Session::Jingle* jingle,
                               const IMPeerId& peerId) {
    if (currentSid.isEmpty()) {
        return false;
    }

    auto& from = session->remote();
    if (from.server().starts_with("conference.")) {
        return false;
    }

    auto sId = qstring(session->sid());
    qDebug() << __func__ << "sId:" << sId;

    ortc::OJingleContentFile cfile;
    cfile.sdpType = ortc::JingleSdpType::Offer;
    parse(jingle, cfile);

    for (auto& f : cfile.contents) {
        File* file0 = new File{.id = qstring(f.id),
                               .sId = qstring(f.sId),
                               .name = qstring(f.name),
                               .path = {},
                               .size = (quint64)f.size,
                               .status = FileStatus::INITIALIZING,
                               .direction = FileDirection::RECEIVING};
        for (auto h : fileHandlers) {
            h->onFileRequest(peerId.toFriendId(), *file0);
        }
        // 创建session
        auto s = new IMFileSession(sId, session, peerId, this, file0);
        m_fileSessionMap.insert(sId, s);
    }

    return true;
}

bool IMFile::handleIq(const gloox::IQ& iq) {
    const auto* ibb = iq.findExtension<gloox::InBandBytestream::IBB>(gloox::ExtIBB);
    if (!ibb) {
        return false;
    }

    IMContactId friendId(qstring(iq.from().bare()));
    qDebug() << __func__ << QString("IBB stream id:%1").arg(qstring(ibb->sid()));

    switch (ibb->type()) {
        case gloox::InBandBytestream::IBBOpen: {
            qDebug() << __func__ << QString("Open");
            break;
        }
        case gloox::InBandBytestream::IBBData: {
            auto sId = qstring(ibb->sid());

            qDebug() << __func__ << QString("Data seq:%1").arg(ibb->seq());
            for (auto h : fileHandlers) {
                h->onFileRecvChunk(friendId.toString(), sId, ibb->seq(), ibb->data());
            }

            break;
        }
        case gloox::InBandBytestream::IBBClose: {
            qDebug() << __func__ << QString("Close");
            auto fileId = qstring(ibb->sid());
            for (auto& k : m_fileSessionMap.keys()) {
                auto ss = m_fileSessionMap.value(k);
                auto file = ss->getFile();
                if (file->id == fileId) {
                    ss->getJingleSession()->sessionTerminate(new gloox::Jingle::Session::Reason(
                            gloox::Jingle::Session::Reason::Success));
                    m_fileSessionMap.remove(k);
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
    auto sId = qstring(session->sid());

    if (currentSid.isEmpty()) {
        return false;
    }
    clearSessionInfo(sId);
    currentSid.clear();
    return true;
}

}  // namespace lib::messenger
