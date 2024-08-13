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

#include "profileform.h"
#include <src/nexus.h>
#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QImageReader>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMap>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QWindow>
#include "lib/settings/translator.h"
#include "src/core/core.h"
#include "src/lib/settings/style.h"
#include "src/model/profile/iprofileinfo.h"
#include "src/persistence/profile.h"
#include "src/persistence/profilelocker.h"
#include "src/persistence/settings.h"
#include "src/widget/contentlayout.h"
#include "src/widget/form/setpassworddialog.h"
#include "src/widget/form/settingswidget.h"
#include "src/widget/gui.h"
#include "src/widget/maskablepixmapwidget.h"
#include "src/widget/tool/croppinglabel.h"
#include "src/widget/widget.h"
#include "ui_profileform.h"

static const QMap<IProfileInfo::SetAvatarResult, QString> SET_AVATAR_ERROR = {
        {IProfileInfo::SetAvatarResult::CanNotOpen, ProfileForm::tr("Unable to open this file.")},
        {IProfileInfo::SetAvatarResult::CanNotRead, ProfileForm::tr("Unable to read this image.")},
        {IProfileInfo::SetAvatarResult::TooLarge,
         ProfileForm::tr("The supplied image is too large.\nPlease use another image.")},
        {IProfileInfo::SetAvatarResult::EmptyPath, ProfileForm::tr("Empty path is unavaliable")},
};

static const QMap<IProfileInfo::RenameResult, QPair<QString, QString>> RENAME_ERROR = {
        {IProfileInfo::RenameResult::Error,
         {ProfileForm::tr("Failed to rename"),
          ProfileForm::tr("Couldn't rename the profile to \"%1\"")}},
        {IProfileInfo::RenameResult::ProfileAlreadyExists,
         {ProfileForm::tr("Profile already exists"),
          ProfileForm::tr("A profile named \"%1\" already exists.")}},
        {IProfileInfo::RenameResult::EmptyName,
         {ProfileForm::tr("Empty name"), ProfileForm::tr("Empty name is unavaliable")}},
};

static const QMap<IProfileInfo::SaveResult, QPair<QString, QString>> SAVE_ERROR = {
        {
                IProfileInfo::SaveResult::NoWritePermission,
                {ProfileForm::tr("Location not writable", "Title of permissions popup"),
                 ProfileForm::tr("You do not have permission to write that location. Choose "
                                 "another, or cancel the save dialog.",
                                 "text of permissions popup")},
        },
        {IProfileInfo::SaveResult::Error,
         {ProfileForm::tr("Failed to copy file"),
          ProfileForm::tr("The file you chose could not be written to.")}},
        {IProfileInfo::SaveResult::EmptyPath,
         {ProfileForm::tr("Empty path"), ProfileForm::tr("Empty path is unavaliable")}},
};

static const QPair<QString, QString> CAN_NOT_CHANGE_PASSWORD = {
        ProfileForm::tr("Couldn't change password"),
        ProfileForm::tr("Couldn't change password on the database, "
                        "it might be corrupted or use the old password.")};

ProfileForm::ProfileForm(IProfileInfo* profileInfo, QWidget* parent)
        : QWidget{parent}, qr{nullptr}, profileInfo{profileInfo} {
    bodyUI = new Ui::IdentitySettings;
    bodyUI->setupUi(this);

    const uint32_t maxNameLength = 1024;  // TODO tox_max_name_length();
    const QString toolTip = tr("Tox user names cannot exceed %1 characters.").arg(maxNameLength);
    bodyUI->userNameLabel->setToolTip(toolTip);
    bodyUI->userName->setMaxLength(static_cast<int>(maxNameLength));

    // tox
    toxId = new ClickableTE();
    toxId->setFont(Style::getFont(Style::Small));
    toxId->setToolTip(bodyUI->toxId->toolTip());

    QVBoxLayout* toxIdGroup = qobject_cast<QVBoxLayout*>(bodyUI->toxGroup->layout());
    delete toxIdGroup->replaceWidget(bodyUI->toxId, toxId);  // Original toxId is in heap, delete it
    bodyUI->toxId->hide();

    profilePicture = new MaskablePixmapWidget(this, QSize(64, 64), ":/img/avatar_mask.svg");
    profilePicture->setPixmap(QPixmap(":/img/contact_dark.svg"));
    profilePicture->setContextMenuPolicy(Qt::CustomContextMenu);
    profilePicture->setClickable(true);
    profilePicture->setObjectName("selfAvatar");
    profilePicture->installEventFilter(this);
    profilePicture->setAccessibleName("Profile avatar");
    profilePicture->setAccessibleDescription("Set a profile avatar shown to all contacts");

    setStyleSheet(Style::getStylesheet("window/profile.css"));

    connect(profilePicture, &MaskablePixmapWidget::clicked, this, &ProfileForm::onAvatarClicked);
    connect(profilePicture, &MaskablePixmapWidget::customContextMenuRequested, this,
            &ProfileForm::showProfilePictureContextMenu);
    QHBoxLayout* publicGrouplayout = qobject_cast<QHBoxLayout*>(bodyUI->publicGroup->layout());
    publicGrouplayout->insertWidget(0, profilePicture);
    publicGrouplayout->insertSpacing(1, 7);

    timer.setInterval(750);
    timer.setSingleShot(true);
    connect(&timer, &QTimer::timeout, this, [this]() {
        // QString x = bodyUI->toxIdLabel->text().replace(" ✔");
        // bodyUI->toxIdLabel->setText(x);
        hasCheck = false;
    });

    connect(bodyUI->toxIdLabel, &CroppingLabel::clicked, this, &ProfileForm::copyIdClicked);
    connect(toxId, &ClickableTE::clicked, this, &ProfileForm::copyIdClicked);
    profileInfo->connectTo_idChanged(this, [this](const ToxId& id) { setToxId(id); });
    connect(bodyUI->userName, &QLineEdit::editingFinished, this, &ProfileForm::onUserNameEdited);
    connect(bodyUI->statusMessage, &QLineEdit::editingFinished, this,
            &ProfileForm::onStatusMessageEdited);
    connect(bodyUI->exportButton, &QPushButton::clicked, this, &ProfileForm::onExportClicked);
    connect(bodyUI->logoutButton, &QPushButton::clicked, this, &ProfileForm::onLogoutClicked);
    connect(bodyUI->exitButton, &QPushButton::clicked, this, &ProfileForm::onExitClicked);
    //    connect(bodyUI->deleteButton, &QPushButton::clicked, this, &ProfileForm::onDeleteClicked);
    //    connect(bodyUI->deletePassButton, &QPushButton::clicked,
    //            this, &ProfileForm::onDeletePassClicked);
    //    connect(bodyUI->deletePassButton, &QPushButton::clicked,
    //            this, &ProfileForm::setPasswordButtonsText);
    //    connect(bodyUI->changePassButton, &QPushButton::clicked,
    //            this, &ProfileForm::onChangePassClicked);
    //    connect(bodyUI->changePassButton, &QPushButton::clicked,
    //            this, &ProfileForm::setPasswordButtonsText);
    connect(bodyUI->saveQr, &QPushButton::clicked, this, &ProfileForm::onSaveQrClicked);
    connect(bodyUI->copyQr, &QPushButton::clicked, this, &ProfileForm::onCopyQrClicked);

    profileInfo->connectTo_usernameChanged(
            this, [this](const QString& val) { bodyUI->userName->setText(val); });
    profileInfo->connectTo_statusMessageChanged(
            this, [this](const QString& val) { bodyUI->statusMessage->setText(val); });

    for (QComboBox* cb : findChildren<QComboBox*>()) {
        cb->installEventFilter(this);
        cb->setFocusPolicy(Qt::StrongFocus);
    }
    connect(Nexus::getProfile(),&Profile::selfAvatarChanged,this,&ProfileForm::onSelfAvatarLoaded);
    
    retranslateUi();
    settings::Translator::registerHandler(std::bind(&ProfileForm::retranslateUi, this), this);
}

void ProfileForm::prFileLabelUpdate() {
    const QString name = profileInfo->getUsername();
    bodyUI->prFileLabel->setText(tr("Current profile: ") + name + ".tox");
}

ProfileForm::~ProfileForm() {
    settings::Translator::unregister(this);
    delete bodyUI;
}

bool ProfileForm::isShown() const {
    if (profilePicture->isVisible()) {
        window()->windowHandle()->alert(0);
        return true;
    }

    return false;
}

void ProfileForm::showTo(ContentLayout* contentLayout) {
    auto idx = contentLayout->indexOf(this);
    if (idx < 0) {
        contentLayout->addWidget(this);
    }
    contentLayout->setCurrentWidget(this);

    prFileLabelUpdate();
    bool portable = Settings::getInstance().getMakeToxPortable();
    QString defaultPath = QDir(Settings::getInstance().getSettingsDirPath()).path().trimmed();
    QString appPath = QApplication::applicationDirPath();
    QString dirPath = portable ? appPath : defaultPath;

    QString dirPrLink = tr("Current profile location: %1")
                                .arg(QString("<a href=\"file://%1\">%1</a>").arg(dirPath));

    bodyUI->dirPrLink->setText(dirPrLink);
    bodyUI->dirPrLink->setOpenExternalLinks(true);
    bodyUI->dirPrLink->setTextInteractionFlags(Qt::LinksAccessibleByMouse |
                                               Qt::TextSelectableByMouse);
    bodyUI->dirPrLink->setMaximumSize(bodyUI->dirPrLink->sizeHint());
    bodyUI->userName->setFocus();
    bodyUI->userName->selectAll();
}

bool ProfileForm::eventFilter(QObject* object, QEvent* event) {
    if (object == static_cast<QObject*>(profilePicture) &&
        event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton) return true;
    }
    return false;
}

void ProfileForm::showEvent(QShowEvent* e) {
    auto avt = profileInfo->getAvatar();
    onSelfAvatarLoaded(avt);

    bodyUI->userName->setText(profileInfo->getDisplayName());
}

void ProfileForm::showProfilePictureContextMenu(const QPoint& point) {
    const QPoint pos = profilePicture->mapToGlobal(point);

    QMenu contextMenu;
    const QIcon icon = style()->standardIcon(QStyle::SP_DialogCancelButton);
    const QAction* removeAction = contextMenu.addAction(icon, tr("Remove"));
    const QAction* selectedItem = contextMenu.exec(pos);

    if (selectedItem == removeAction) {
        profileInfo->removeAvatar();
    }
}

void ProfileForm::copyIdClicked() {
    profileInfo->copyId();
    if (!hasCheck) {
        bodyUI->toxIdLabel->setText(bodyUI->toxIdLabel->text());  // TODO: + " ✔"
        hasCheck = true;
    }

    timer.start();
}

void ProfileForm::onUserNameEdited() { profileInfo->setUsername(bodyUI->userName->text()); }

void ProfileForm::onStatusMessageEdited() {
    profileInfo->setStatusMessage(bodyUI->statusMessage->text());
}

void ProfileForm::onSelfAvatarLoaded(const QPixmap& pic) { profilePicture->setPixmap(pic); }

void ProfileForm::setToxId(const ToxId& id) {
    QString idString = id.toString();
    qDebug() << "setToxId id:" << idString;
    toxId->setText(idString);
    setQrCode(idString);
}

void ProfileForm::setQrCode(const QString& id) {
    qr = std::make_unique<QRWidget>();
    qr->setQRData(id);
    bodyUI->qrCode->setPixmap(QPixmap::fromImage(qr->getImage()->scaledToWidth(150)));
}

QString ProfileForm::getSupportedImageFilter() {
    QString res;
    for (auto type : QImageReader::supportedImageFormats()) {
        res += QString("*.%1 ").arg(QString(type));
    }

    return tr("Images (%1)", "filetype filter").arg(res.left(res.size() - 1));
}

void ProfileForm::onAvatarClicked() {
    const QString filter = getSupportedImageFilter();
    const QString path = QFileDialog::getOpenFileName(Q_NULLPTR, tr("Choose a profile picture"),
                                                      QDir::homePath(), filter, nullptr);

    if (path.isEmpty()) {
        return;
    }
    
    const IProfileInfo::SetAvatarResult result = profileInfo->setAvatar(path);
    if (result == IProfileInfo::SetAvatarResult::OK) {   
        return;
    }

    GUI::showError(tr("Error"), SET_AVATAR_ERROR[result]);
}

void ProfileForm::onRenameClicked() {
    const QString cur = profileInfo->getUsername();
    const QString title = tr("Rename \"%1\"", "renaming a profile").arg(cur);
    const QString name = QInputDialog::getText(this, title, title + ":");
    if (name.isEmpty()) {
        return;
    }

    const IProfileInfo::RenameResult result = profileInfo->renameProfile(name);
    if (result == IProfileInfo::RenameResult::OK) {
        return;
    }
    const QPair<QString, QString> error = RENAME_ERROR[result];
    GUI::showError(error.first, error.second.arg(name));
    prFileLabelUpdate();
}

void ProfileForm::onExportClicked() {
    const QString current = profileInfo->getUsername() + Core::TOX_EXT;
    //: save dialog title
    const QString path = QFileDialog::getSaveFileName(Q_NULLPTR, tr("Export profile"), current,
                                                      //: save dialog filter
                                                      tr("Tox save file (*.tox)"));
    if (path.isEmpty()) {
        return;
    }

    const IProfileInfo::SaveResult result = profileInfo->exportProfile(path);
    if (result == IProfileInfo::SaveResult::OK) {
        return;
    }

    const QPair<QString, QString> error = SAVE_ERROR[result];
    GUI::showWarning(error.first, error.second);
}

void ProfileForm::onDeleteClicked() {
    const QString title = tr("Really delete profile?", "deletion confirmation title");
    const QString question =
            tr("Are you sure you want to delete this profile?", "deletion confirmation text");
    if (!GUI::askQuestion(title, question)) {
        return;
    }

    const QStringList manualDeleteFiles = profileInfo->removeProfile();
    if (manualDeleteFiles.empty()) {
        return;
    }

    //: deletion failed text part 1
    QString message = tr("The following files could not be deleted:") + "\n\n";
    for (const QString& file : manualDeleteFiles) {
        message += file + "\n";
    }

    //: deletion failed text part 2
    message += "\n" + tr("Please manually remove them.");

    GUI::showError(tr("Files could not be deleted!", "deletion failed title"), message);
}

void ProfileForm::onLogoutClicked() { profileInfo->logout(); }

void ProfileForm::onExitClicked() { profileInfo->exit(); }

void ProfileForm::setPasswordButtonsText() {
    //    if (profileInfo->isEncrypted()) {
    //        bodyUI->changePassButton->setText(tr("Change password", "button text"));
    //        bodyUI->deletePassButton->setVisible(true);
    //    } else {
    //        bodyUI->changePassButton->setText(tr("Set profile password", "button text"));
    //        bodyUI->deletePassButton->setVisible(false);
    //    }
}

void ProfileForm::onCopyQrClicked() {
    if (!qr) return;
    profileInfo->copyQr(*qr->getImage());
}

void ProfileForm::onSaveQrClicked() {
    const QString current = profileInfo->getUsername() + ".png";

    const QString path =
            QFileDialog::getSaveFileName(Q_NULLPTR, tr("Save", "save qr image"), current,
                                         tr("Save QrCode (*.png)", "save dialog filter"));
    if (path.isEmpty()) {
        return;
    }

    const IProfileInfo::SaveResult result = profileInfo->saveQr(*qr->getImage(), path);
    if (result == IProfileInfo::SaveResult::OK) {
        return;
    }

    const QPair<QString, QString> error = SAVE_ERROR[result];
    GUI::showWarning(error.first, error.second);
}

void ProfileForm::onDeletePassClicked() {
    if (!profileInfo->isEncrypted()) {
        GUI::showInfo(tr("Nothing to remove"), tr("Your profile does not have a password!"));
        return;
    }

    const QString title = tr("Really delete password?", "deletion confirmation title");
    //: deletion confirmation text
    const QString body = tr("Are you sure you want to delete your password?");
    if (!GUI::askQuestion(title, body)) {
        return;
    }

    if (!profileInfo->deletePassword()) {
        GUI::showInfo(CAN_NOT_CHANGE_PASSWORD.first, CAN_NOT_CHANGE_PASSWORD.second);
    }
}

void ProfileForm::onChangePassClicked() {
    const QString title = tr("Please enter a new password.");
    SetPasswordDialog* dialog = new SetPasswordDialog(title, QString{}, nullptr);
    if (dialog->exec() == QDialog::Rejected) {
        return;
    }

    QString newPass = dialog->getPassword();
    if (!profileInfo->setPassword(newPass)) {
        GUI::showInfo(CAN_NOT_CHANGE_PASSWORD.first, CAN_NOT_CHANGE_PASSWORD.second);
    }
}

void ProfileForm::retranslateUi() {
    bodyUI->retranslateUi(this);
    setPasswordButtonsText();
    // We have to add the toxId tooltip here and not in the .ui or Qt won't know how to translate it
    // dynamically
    toxId->setToolTip(
            tr("This bunch of characters tells other Tox clients how to contact "
               "you.\nShare it with your friends to communicate.\n\n"
               "This ID includes the NoSpam code (in blue), and the checksum (in gray)."));
}
