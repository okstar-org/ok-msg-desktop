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

#ifndef IDENTITYFORM_H
#define IDENTITYFORM_H

#include <QLabel>
#include <QLineEdit>
#include <QTimer>
#include <QVBoxLayout>


namespace Ui {
class IdentitySettings;
}

namespace lib::ui {
class MaskablePixmapWidget;
class CroppingLabel;
class QRWidget;
}

namespace module::im {

class ContentLayout;

class IProfileInfo;
class ToxId;

class ClickableTE : public QLabel {
    Q_OBJECT
public:
signals:
    void clicked();

protected:
    virtual void mouseReleaseEvent(QMouseEvent*) final override {
        emit clicked();
    }
};

/**
 * 个人信息表单界面
 */
class ProfileForm : public QWidget {
    Q_OBJECT
public:
    ProfileForm(IProfileInfo* profileInfo, QWidget* parent = nullptr);
    ~ProfileForm();

    void showTo(ContentLayout* contentLayout);
    bool isShown() const;

public slots:
    void onSelfAvatarLoaded(const QPixmap& pic);
    void onLogoutClicked();
    void onExitClicked();

protected:
    virtual bool eventFilter(QObject* object, QEvent* event) override;
    virtual void showEvent(QShowEvent* e) override;
    void contextMenuEvent(QContextMenuEvent* e);

private slots:
    void setPasswordButtonsText();

    void copyIdClicked();
    void onNicknameEdited();
    void onExportClicked();

    void onCopyQrClicked();
    void onSaveQrClicked();
    void onDeletePassClicked();
    void onChangePassClicked();
    void onAvatarClicked();
    void showProfilePictureContextMenu(const QPoint& point);
    void showQRCode();

private:
    void retranslateUi();

    void refreshProfiles();
    static QString getSupportedImageFilter();

private:
    Ui::IdentitySettings* bodyUI;
    lib::ui::MaskablePixmapWidget* profilePicture;
    lib::ui::QRWidget* qr;
    IProfileInfo* profileInfo;
};
}  // namespace module::im
#endif
