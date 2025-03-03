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

#include "profileimporter.h"

#include <QApplication>
#include <QFileDialog>

#include <QPushButton>

#include "src/base/MessageBox.h"
#include "src/core/core.h"
#include "src/nexus.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
namespace module::im {

/**
 * @class ProfileImporter
 * @brief This class provides the ability to import profile.
 * @note This class can be used before @a Nexus instance will be created,
 * consequently it can't use @a GUI class. Therefore it should use QMessageBox
 * to create dialog forms.
 */

ProfileImporter::ProfileImporter(QWidget* parent) : QWidget(parent) {}

/**
 * @brief Show a file dialog. Selected file will be imported as Tox profile.
 * @return True, if the import was succesful. False otherwise.
 */
bool ProfileImporter::importProfile() {
    QString title = tr("Import profile", "import dialog title");
    QString filter = tr("Tox save file (*.tox)", "import dialog filter");
    QString dir = QDir::homePath();

    // TODO: Change all QFileDialog instances across project to use
    // this instead of Q_NULLPTR. Possibly requires >Qt 5.9 due to:
    // https://bugreports.qt.io/browse/QTBUG-59184
    QString path = QFileDialog::getOpenFileName(Q_NULLPTR, title, dir, filter);

    return importProfile(path);
}

/**
 * @brief Asks the user a question with Yes/No choices.
 * @param title Title of question window.
 * @param message Text in question window.
 * @return True if the answer is positive, false otherwise.
 */
bool ProfileImporter::askQuestion(QString title, QString message) {
    QMessageBox::Icon icon = QMessageBox::Warning;
    QMessageBox box(icon, title, message, QMessageBox::NoButton, this);
    QPushButton* pushButton1 = box.addButton(QApplication::tr("Yes"), QMessageBox::AcceptRole);
    QPushButton* pushButton2 = box.addButton(QApplication::tr("No"), QMessageBox::RejectRole);
    box.setDefaultButton(pushButton2);
    box.setEscapeButton(pushButton2);

    box.exec();
    return box.clickedButton() == pushButton1;
}

/**
 * @brief Try to import Tox profile.
 * @param path Path to Tox profile.
 * @return True, if the import was succesful. False otherwise.
 */
bool ProfileImporter::importProfile(const QString& path) {
    if (path.isEmpty()) return false;

    QFileInfo info(path);
    if (!info.exists()) {
        ok::base::MessageBox::warning(this, tr("File doesn't exist"), tr("Profile doesn't exist"),
                                      QMessageBox::Ok);
        return false;
    }

    QString profile = info.completeBaseName();

    if (info.suffix() != "tox") {
        ok::base::MessageBox::warning(this, tr("Ignoring non-Tox file", "popup title"),
                                      tr("Warning: You have chosen a file that is not a "
                                         "Tox save file; ignoring.",
                                         "popup text"),
                                      QMessageBox::Ok);
        return false;  // ingore importing non-tox file
    }

    QString settingsPath = Nexus::getProfile()->getSettings()->getSettingsDirPath();
    QString profilePath = QDir(settingsPath).filePath(profile + Core::TOX_EXT);

    if (QFileInfo(profilePath).exists()) {
        QString title = tr("Profile already exists", "import confirm title");
        QString message = tr("A profile named \"%1\" already exists. "
                             "Do you want to erase it?",
                             "import confirm text")
                                  .arg(profile);
        bool erase = askQuestion(title, message);

        if (!erase) return false;  // import canelled

        QFile(profilePath).remove();
    }

    QFile::copy(path, profilePath);
    // no good way to update the ui from here... maybe we need a Widget:refreshUi() function...
    // such a thing would simplify other code as well I believe
    QMessageBox::information(this, tr("Profile imported"),
                             tr("%1.tox was successfully imported").arg(profile), QMessageBox::Ok);

    return true;  // import successfull
}
}  // namespace module::im