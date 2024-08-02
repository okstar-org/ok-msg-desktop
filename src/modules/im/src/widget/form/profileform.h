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
#include "src/widget/qrwidget.h"

class ContentLayout;
class CroppingLabel;
class IProfileInfo;
class MaskablePixmapWidget;

namespace Ui {
class IdentitySettings;
}
class ToxId;
class ClickableTE : public QLabel {
    Q_OBJECT
public:
signals:
    void clicked();

protected:
    virtual void mouseReleaseEvent(QMouseEvent*) final override { emit clicked(); }
};

class ProfileForm : public QWidget {
    Q_OBJECT
public:
    ProfileForm(IProfileInfo* profileInfo, QWidget* parent = nullptr);
    ~ProfileForm();

    void showTo(ContentLayout* contentLayout);
    bool isShown() const;
    void setQrCode(const QString& id);

public slots:
    void onSelfAvatarLoaded(const QPixmap& pic);
    void onLogoutClicked();
    void onExitClicked();

protected:
    virtual bool eventFilter(QObject* object, QEvent* event) override;
    virtual void showEvent(QShowEvent* e) override;

private slots:
    void setPasswordButtonsText();
    void setToxId(const ToxId& id);
    void copyIdClicked();
    void onUserNameEdited();
    void onStatusMessageEdited();
    void onRenameClicked();
    void onExportClicked();
    void onDeleteClicked();
    void onCopyQrClicked();
    void onSaveQrClicked();
    void onDeletePassClicked();
    void onChangePassClicked();
    void onAvatarClicked();
    void showProfilePictureContextMenu(const QPoint& point);

private:
    void retranslateUi();
    void prFileLabelUpdate();
    void refreshProfiles();
    static QString getSupportedImageFilter();

private:
    Ui::IdentitySettings* bodyUI;
    MaskablePixmapWidget* profilePicture;
    QTimer timer;
    bool hasCheck = false;
    std::unique_ptr<QRWidget> qr;
    ClickableTE* toxId;
    IProfileInfo* profileInfo;
};

#endif
