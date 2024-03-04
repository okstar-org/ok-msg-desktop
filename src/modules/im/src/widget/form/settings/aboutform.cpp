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
#include "ui_aboutsettings.h"

#include "src/net/updatecheck.h"
#include "src/persistence/profile.h"
#include "src/persistence/settings.h"
#include "src/widget/style.h"
#include "src/widget/tool/recursivesignalblocker.h"
#include "lib/settings/translator.h"

//#include <tox/tox.h>

#include <QDebug>
#include <QDesktopServices>
#include <QPushButton>
#include <QTimer>

#include <memory>

// index of UI in the QStackedWidget
enum class updateIndex
{
    available = 0,
    upToDate = 1,
    failed = 2
};

/**
 * @class AboutForm
 *
 * This form contains information about qTox and libraries versions, external
 * links and licence text. Shows progress during an update.
 */

/**
 * @brief Constructor of AboutForm.
 */
AboutForm::AboutForm(UpdateCheck* updateCheck)
    : GenericForm(QPixmap(":/img/settings/general.png"))
    , bodyUI(new Ui::AboutSettings)
    , progressTimer(new QTimer(this))
    , updateCheck(updateCheck)
{
    bodyUI->setupUi(this);

    // block all child signals during initialization
    const RecursiveSignalBlocker signalBlocker(this);

    replaceVersions();

    if (QString(GIT_VERSION).indexOf(" ") > -1)
        bodyUI->gitVersion->setOpenExternalLinks(false);

    eventsInit();
    settings::Translator::registerHandler(std::bind(&AboutForm::retranslateUi, this), this);
}

/**
 * @brief Update versions and links.
 *
 * Update commit hash if built with git, show author and known issues info
 * It also updates qTox, toxcore and Qt versions.
 */
void AboutForm::replaceVersions()
{
    // TODO: When we finally have stable releases: build-in a way to tell
    bodyUI->youAreUsing->setText(tr("You are using qTox version %1.").arg(QString(GIT_DESCRIBE)));

#if UPDATE_CHECK_ENABLED
    if (updateCheck != nullptr) {
        connect(updateCheck, &UpdateCheck::updateAvailable, this, &AboutForm::onUpdateAvailable);
        connect(updateCheck, &UpdateCheck::upToDate, this, &AboutForm::onUpToDate);
        connect(updateCheck, &UpdateCheck::updateCheckFailed, this, &AboutForm::onUpdateCheckFailed);
    } else {
        qWarning() << "AboutForm passed null UpdateCheck!";
    }
#else
    qDebug() << "AboutForm not showing updates, qTox built without UPDATE_CHECK";
#endif

    QString commitLink = "https://gitee.com/okstar-org/ok-edu-desktop/commit/" + QString(GIT_VERSION);
    bodyUI->gitVersion->setText(tr("Commit hash: %1").arg(createLink(commitLink, QString(GIT_VERSION))));
    bodyUI->qtVersion->setText(tr("Qt version: %1").arg(QT_VERSION_STR));

    QString issueBody = QString("##### Brief Description\n\n"
                                "OS: %1\n"
                                "version: %2\n"
                                "Commit hash: %3\n"
                                "Qt: %4\n")
                            .arg(QSysInfo::prettyProductName(),
                                 GIT_DESCRIBE,
                                 GIT_VERSION,
                                 QT_VERSION_STR);

    issueBody.replace("#", "%23").replace(":", "%3A");

    bodyUI->knownIssues->setText(
        tr("如果您在使用OkEDU®过程中遇到任何问题,"
           " 都可以在 %2"
           " 上向我们的提交%1，团队将会第一时间处理。",
           "`%1` is replaced by translation of `Issue`"
           "\n`%2` is replaced by translation of `Gitee`")
            .arg(createLink(
                "https://gitee.com/okstar-org/ok-edu-desktop/"
                "issues",
                tr("Issue", "Replaces `%1` in the `If you encounter…`")))
            .arg(createLink(
                "https://gitee.com",
                tr("Gitee", "Replaces `%2` in the `A list of all known…`"))));

//    bodyUI->clickToReport->setText(
//        createLink("https://github.com/qTox/qTox/issues/new?body=" + QUrl(issueBody).toEncoded(),
//                   QString("<b>%1</b>").arg(tr("Click here to report a bug."))));


    QString authorInfo =
        QString("<p>%1 &nbsp; %2</p>")
            .arg(tr("Original author: %1").arg(createLink("https://gitee.com/okstar-org", "okstar.org")))
            .arg(
                tr("See a full list of %1 at Github",
                   "`%1` is replaced with translation of word `contributors`")
                    .arg(createLink("https://gitee.com/okstar-org/ok-edu-desktop/contributors?ref=master",
                                    tr("contributors", "Replaces `%1` in `See a full list of…`"))));

    bodyUI->authorInfo->setText(authorInfo);
}

void AboutForm::onUpdateAvailable(QString latestVersion, QUrl link)
{
    QObject::disconnect(linkConnection);
    linkConnection = connect(bodyUI->updateAvailableButton, &QPushButton::clicked,
                             [link]() { QDesktopServices::openUrl(link); });
    bodyUI->updateStack->setCurrentIndex(static_cast<int>(updateIndex::available));
}

void AboutForm::onUpToDate()
{
    bodyUI->updateStack->setCurrentIndex(static_cast<int>(updateIndex::upToDate));
}

void AboutForm::onUpdateCheckFailed()
{
    bodyUI->updateStack->setCurrentIndex(static_cast<int>(updateIndex::failed));
}

/**
 * @brief Creates hyperlink with specific style.
 * @param path The URL of the page the link goes to.
 * @param text Text, which will be clickable.
 * @return Hyperlink to paste.
 */
QString AboutForm::createLink(QString path, QString text) const
{
    return QString::fromUtf8(
               "<a href=\"%1\" style=\"text-decoration: underline; color:%2;\">%3</a>")
        .arg(path, Style::getColor(Style::Link).name(), text);
}

AboutForm::~AboutForm()
{
    settings::Translator::unregister(this);
    delete bodyUI;
}

/**
 * @brief Retranslate all elements in the form.
 */
void AboutForm::retranslateUi()
{
    bodyUI->retranslateUi(this);
    replaceVersions();
}
