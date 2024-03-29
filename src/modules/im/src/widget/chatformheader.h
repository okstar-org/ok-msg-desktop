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

class MaskablePixmapWidget;
class QVBoxLayout;
class CroppingLabel;
class QPushButton;
class QToolButton;
class CallConfirmWidget;

class ChatFormHeader : public QWidget
{
    Q_OBJECT
public:
    enum class CallButtonState {
        Disabled = 0,    // Grey
        Avaliable = 1,   // Green
        InCall = 2,      // Red
        Outgoing = 3,    // Yellow
        Incoming = 4,    // Yellow
    };
    enum class ToolButtonState {
        Disabled = 0,    // Grey
        Off = 1,         // Green
        On = 2,          // Red
    };
    enum Mode {
        None = 0,
        Audio = 1,
        Video = 2,
        AV = Audio | Video
    };

    ChatFormHeader(QWidget* parent = nullptr);
    ~ChatFormHeader();

    void setName(const QString& newName);
    void setMode(Mode mode);

    void showOutgoingCall(bool video);
    void createCallConfirm(bool video);
    void showCallConfirm();
    void removeCallConfirm();

    void updateCallButtons(bool online, bool audio, bool video = false);
    void updateMuteMicButton(bool active, bool inputMuted);
    void updateMuteVolButton(bool active, bool outputMuted);

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

    void nameChanged(const QString& name);

    void callAccepted();
    void callRejected();

private slots:
    void retranslateUi();
    void updateButtonsView();

private:
    Mode mode;
    MaskablePixmapWidget* avatar;
    QVBoxLayout* headTextLayout;
    CroppingLabel* nameLabel;

    QPushButton* callButton;
    QPushButton* videoButton;
    QPushButton* volButton;
    QPushButton* micButton;

    CallButtonState callState;
    CallButtonState videoState;
    ToolButtonState volState;
    ToolButtonState micState;

    std::unique_ptr<CallConfirmWidget> callConfirm;
};

#endif // CHAT_FORM_HEADER
