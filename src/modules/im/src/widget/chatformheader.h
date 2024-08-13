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

#ifndef CHAT_FORM_HEADER
#define CHAT_FORM_HEADER

#include <QWidget>

#include <memory>

#include <src/core/FriendId.h>
#include <src/core/contactid.h>
#include "src/model/status.h"

class MaskablePixmapWidget;
class QVBoxLayout;
class CroppingLabel;
class QPushButton;
class QToolButton;
class CallConfirmWidget;
class ToxPeer;
class Contact;
class QLabel;

class ChatFormHeader : public QWidget {
    Q_OBJECT
public:
    enum class CallButtonState {
        Disabled = 0,   // Grey
        Avaliable = 1,  // Green
        InCall = 2,     // Red
        Outgoing = 3,   // Yellow
        Incoming = 4,   // Yellow
    };
    enum class ToolButtonState {
        Disabled = 0,  // Grey
        Off = 1,       // Green
        On = 2,        // Red
    };
    enum Mode { None = 0, Audio = 1, Video = 2, AV = Audio | Video };

    ChatFormHeader(const ContactId& contactId, QWidget* parent = nullptr);
    ~ChatFormHeader();

    void setContact(const Contact* contact);
    void removeContact();
    const Contact* getContact() const;

    void setName(const QString& newName);
    void setMode(Mode mode);

    void showOutgoingCall(bool video);

    void createCallConfirm(const ToxPeer& peer, bool video, QString& displayedName);
    void showCallConfirm();
    void removeCallConfirm();

    void updateMuteMicButton(bool active, bool inputMuted);
    void updateMuteVolButton(bool active, bool outputMuted);

    void updateCallButtons();
    void updateCallButtons(Status::Status status);
    void updateCallButtons(bool online, bool audio, bool video = false);

    void updateMuteMicButton();
    void updateMuteVolButton();

    void showCallConfirm(const ToxPeer& peerId, bool video, const QString& displayedName);

    void setAvatar(const QPixmap& img);
    QSize getAvatarSize() const;

    void reloadTheme();

    // TODO: Remove
    void addWidget(QWidget* widget, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());
    void addLayout(QLayout* layout);
    void addStretch();

signals:
    void callTriggered();
    void videoCallTriggered();
    void micMuteToggle();
    void volMuteToggle();

    void callAccepted(const ToxPeer& peerId, bool video);
    void callRejected(const ToxPeer& peerId);

private slots:
    void retranslateUi();
    void updateButtonsView();
    void onDisplayedNameChanged(const QString& name);

private:
    void nameChanged(const QString& name);
    void updateContactStatus(Status::Status status);

private:
    // 初始情况下可能先有contactId
    // 后 Friend 对象被构造，通过setContact接口创建出来
    ContactId contactId;
    const Contact* contact = nullptr;
    bool isSelf = false;

    Mode mode;
    MaskablePixmapWidget* avatar;
    QVBoxLayout* headTextLayout;
    CroppingLabel* nameLabel;
    QLabel* statusLabel = nullptr;
    QToolButton* statusIcon = nullptr;

    QPushButton* callButton;
    QPushButton* videoButton;

    CallButtonState callState;
    CallButtonState videoState;

    std::unique_ptr<CallConfirmWidget> callConfirm;
};

#endif  // CHAT_FORM_HEADER
