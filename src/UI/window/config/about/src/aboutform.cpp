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

#include "aboutform.h"

#include <QDebug>
#include <QDesktopServices>
#include <QPushButton>
#include <QTimer>
#include "lib/settings/translator.h"
#include "src/UI/widget/GenericForm.h"
#include "src/base/RecursiveSignalBlocker.h"
#include "src/lib/settings/style.h"

#include <memory>
namespace UI {

// index of UI in the QStackedWidget
enum class updateIndex { available = 0, upToDate = 1, failed = 2 };

/**
 * @class AboutForm
 *
 * This form contains information about qTox and libraries versions, external
 * links and licence text. Shows progress during an update.
 */

/**
 * @brief Constructor of AboutForm.
 */
AboutForm::AboutForm(QWidget* parent)
        : GenericForm(QPixmap(":/img/settings/general.png"), parent)
        , bodyUI(new Ui::AboutSettings)
        , progressTimer(new QTimer(this)) {
    bodyUI->setupUi(this);

    if (QString(GIT_VERSION).indexOf(" ") > -1) bodyUI->gitVersion->setOpenExternalLinks(false);

    QString lic = QString("Copyright (c) 2022 **%1 %2**\n\r"
                          "**%3** is licensed under Mulan PubL v2.\n\r"
                          "You can use this software according to the terms and conditions of the "
                          "Mulan PubL v2.\n\r"
                          "You may obtain a copy of Mulan PubL v2 at:\n\r"
                          "**http://license.coscl.org.cn/MulanPubL-2.0**\n\r"
                          " THIS SOFTWARE IS PROVIDED ON AN \"AS IS\" BASIS,\n\r"
                          " WITHOUT WARRANTIES OF ANY KIND,EITHER EXPRESS OR IMPLIED,\n\r"
                          " INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,\n\r"
                          " MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.\n\r"
                          " See the Mulan PubL v2 for more details.\n\r")
                          .arg(ORGANIZATION_NAME)
                          .arg(ORGANIZATION_DOMAIN)
                          .arg(APPLICATION_NAME);

    bodyUI->license->setMarkdown(lic);

    // block all child signals during initialization
    const ok::base::RecursiveSignalBlocker signalBlocker(this);

    replaceVersions();

    settings::Translator::registerHandler(std::bind(&AboutForm::retranslateUi, this), this);
}

/**
 * @brief Update versions and links.
 *
 * Update commit hash if built with git, show author and known issues info
 * It also updates qTox, toxcore and Qt versions.
 */
void AboutForm::replaceVersions() {
    // TODO: When we finally have stable releases: build-in a way to tell
    bodyUI->youAreUsing->setText(tr("You are using qTox version %1.").arg(QString(GIT_DESCRIBE)));

#if UPDATE_CHECK_ENABLED
    if (updateCheck != nullptr) {
        connect(updateCheck, &UpdateCheck::updateAvailable, this, &AboutForm::onUpdateAvailable);
        connect(updateCheck, &UpdateCheck::upToDate, this, &AboutForm::onUpToDate);
        connect(updateCheck,
                &UpdateCheck::updateCheckFailed,
                this,
                &AboutForm::onUpdateCheckFailed);
    } else {
        qWarning() << "AboutForm passed null UpdateCheck!";
    }
#else
    qDebug() << "AboutForm not showing updates, qTox built without UPDATE_CHECK";
#endif
    QString projectLink = ORGANIZATION_HOME "/" APPLICATION_ALIAS;
    QString commitLink = projectLink + "/commit/" + QString(GIT_VERSION);
    bodyUI->gitVersion->setText(
            tr("Commit hash: %1").arg(createLink(commitLink, QString(GIT_VERSION))));
    bodyUI->qtVersion->setText(tr("Qt version: %1").arg(QT_VERSION_STR));

    QString issueBody =
            QString("##### Brief Description\n\n"
                    "OS: %1\n"
                    "version: %2\n"
                    "Commit hash: %3\n"
                    "Qt: %4\n")
                    .arg(QSysInfo::prettyProductName(), GIT_DESCRIBE, GIT_VERSION, QT_VERSION_STR);

    issueBody.replace("#", "%23").replace(":", "%3A");

    QString issue = QString("%1").arg(createLink(projectLink + "/issues", "Issues"));
    bodyUI->knownIssues->setText(issue);

    QString authorInfo = QString("<p>%1</p>").arg(ORGANIZATION_HOME);
    bodyUI->authorInfo->setText(authorInfo);
}

void AboutForm::onUpdateAvailable(QString latestVersion, QUrl link) {
    QObject::disconnect(linkConnection);
    linkConnection = connect(bodyUI->updateAvailableButton, &QPushButton::clicked, [link]() {
        QDesktopServices::openUrl(link);
    });
    bodyUI->updateStack->setCurrentIndex(static_cast<int>(updateIndex::available));
}

void AboutForm::onUpToDate() {
    bodyUI->updateStack->setCurrentIndex(static_cast<int>(updateIndex::upToDate));
}

void AboutForm::onUpdateCheckFailed() {
    bodyUI->updateStack->setCurrentIndex(static_cast<int>(updateIndex::failed));
}

/**
 * @brief Creates hyperlink with specific style.
 * @param path The URL of the page the link goes to.
 * @param text Text, which will be clickable.
 * @return Hyperlink to paste.
 */
QString AboutForm::createLink(QString path, QString text) const {
    return QString::fromUtf8(
                   "<a href=\"%1\" style=\"text-decoration: underline; color:%2;\">%3</a>")
            .arg(path, Style::getColor(Style::Link).name(), text);
}

AboutForm::~AboutForm() {
    settings::Translator::unregister(this);
    delete bodyUI;
}

/**
 * @brief Retranslate all elements in the form.
 */
void AboutForm::retranslateUi() {
    bodyUI->retranslateUi(this);
    replaceVersions();
}

}  // namespace UI
