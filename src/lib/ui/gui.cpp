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

#include "gui.h"
#include <assert.h>
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDialogButtonBox>
#include <QInputDialog>
#include <QLabel>

#include <QPushButton>
#include <QThread>
#include "application.h"
#include "base/MessageBox.h"

namespace lib::ui {

/**
 * @class GUI
 * @brief Abstracts the GUI from the target backend (DesktopGUI, ...)
 *
 * All the functions exposed here are thread-safe.
 * Prefer calling this class to calling a GUI backend directly.
 *
 * @fn void GUI::resized()
 * @brief Emitted when the GUI is resized on supported platforms.
 */

GUI::GUI(QObject* parent) : QObject(parent) {
    assert(QThread::currentThread() == qApp->thread());
}
GUI::~GUI() {}

/**
 * @brief Returns the singleton instance.
 */
GUI& GUI::getInstance() {
    static GUI gui;
    return gui;
}

/**
 * @brief Change the title of the main window.
 * @param title Titile to set.
 *
 * This is usually always visible to the user.
 */
void GUI::setWindowTitle(const QString& title) {
    if (QThread::currentThread() == qApp->thread()) {
        getInstance()._setWindowTitle(title);
    } else {
        QMetaObject::invokeMethod(&getInstance(), "_setWindowTitle", Qt::BlockingQueuedConnection,
                                  Q_ARG(const QString&, title));
    }
}

/**
 * @brief Reloads the application theme and redraw the window.
 */
void GUI::reloadTheme() {
    if (QThread::currentThread() == qApp->thread()) {
        getInstance()._reloadTheme();
        emit getInstance().themeApplyRequest();
    } else {
        QMetaObject::invokeMethod(&getInstance(), "_reloadTheme", Qt::BlockingQueuedConnection);
    }
}

/**
 * @brief Show some text to the user.
 * @param title Title of information window.
 * @param msg Text in information window.
 */
void GUI::showInfo(const QString& title, const QString& msg) {
    if (QThread::currentThread() == qApp->thread()) {
        getInstance()._showInfo(title, msg);
    } else {
        QMetaObject::invokeMethod(&getInstance(), "_showInfo", Qt::BlockingQueuedConnection,
                                  Q_ARG(const QString&, title), Q_ARG(const QString&, msg));
    }
}

/**
 * @brief Show a warning to the user
 * @param title Title of warning window.
 * @param msg Text in warning window.
 */
void GUI::showWarning(const QString& title, const QString& msg) {
    if (QThread::currentThread() == qApp->thread()) {
        getInstance()._showWarning(title, msg);
    } else {
        QMetaObject::invokeMethod(&getInstance(), "_showWarning", Qt::BlockingQueuedConnection,
                                  Q_ARG(const QString&, title), Q_ARG(const QString&, msg));
    }
}

/**
 * @brief Show an error to the user.
 * @param title Title of error window.
 * @param msg Text in error window.
 */
void GUI::showError(const QString& title, const QString& msg) {
    if (QThread::currentThread() == qApp->thread()) {
        // If the GUI hasn't started yet and we're on the main thread,
        // we still want to be able to show error messages
        getInstance()._showError(title, msg);
    } else {
        QMetaObject::invokeMethod(&getInstance(), "_showError", Qt::BlockingQueuedConnection,
                                  Q_ARG(const QString&, title), Q_ARG(const QString&, msg));
    }
}

/**
 * @brief Asks the user a question with Ok/Cancel or Yes/No buttons.
 * @param title Title of question window.
 * @param msg Text in question window.
 * @param defaultAns If is true, default was positive answer. Negative otherwise.
 * @param warning If is true, we will use a special warning style.
 * @param yesno Show "Yes" and "No" buttons.
 * @return True if the answer is positive, false otherwise.
 */
bool GUI::askQuestion(const QString& title, const QString& msg, bool defaultAns, bool warning,
                      bool yesno) {
    if (QThread::currentThread() == qApp->thread()) {
        return getInstance()._askQuestion(title, msg, defaultAns, warning, yesno);
    } else {
        bool ret;
        QMetaObject::invokeMethod(&getInstance(), "_askQuestion", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(bool, ret), Q_ARG(const QString&, title),
                                  Q_ARG(const QString&, msg), Q_ARG(bool, defaultAns),
                                  Q_ARG(bool, warning), Q_ARG(bool, yesno));
        return ret;
    }
}

/**
 * @brief Asks the user a question.
 *
 * The text for the displayed buttons can be specified.
 * @param title Title of question window.
 * @param msg Text in question window.
 * @param button1 Text of positive button.
 * @param button2 Text of negative button.
 * @param defaultAns If is true, default was positive answer. Negative otherwise.
 * @param warning If is true, we will use a special warning style.
 * @return True if the answer is positive, false otherwise.
 */
bool GUI::askQuestion(const QString& title, const QString& msg, const QString& button1,
                      const QString& button2, bool defaultAns, bool warning) {
    if (QThread::currentThread() == qApp->thread()) {
        return getInstance()._askQuestion(title, msg, button1, button2, defaultAns, warning);
    } else {
        bool ret;
        QMetaObject::invokeMethod(&getInstance(), "_askQuestion", Qt::BlockingQueuedConnection,
                                  Q_RETURN_ARG(bool, ret), Q_ARG(const QString&, title),
                                  Q_ARG(const QString&, msg), Q_ARG(bool, defaultAns),
                                  Q_ARG(bool, warning));
        return ret;
    }
}

void GUI::_setWindowTitle(const QString& title) {
    QWidget* w = getMainWidget();
    if (!w) return;
    if (title.isEmpty())
        w->setWindowTitle("OkMsg");
    else
        w->setWindowTitle("OkMsg - " + title);
}

void GUI::_reloadTheme() {
    qDebug() << __func__;
}

void GUI::_showInfo(const QString& title, const QString& msg) {
    QMessageBox messageBox(QMessageBox::Information, title, msg, QMessageBox::Ok, getMainWidget());
    messageBox.setButtonText(QMessageBox::Ok, QApplication::tr("Ok"));
    messageBox.exec();
}

void GUI::_showWarning(const QString& title, const QString& msg) {
    QMessageBox messageBox(QMessageBox::Warning, title, msg, QMessageBox::Ok, getMainWidget());
    messageBox.setButtonText(QMessageBox::Ok, QApplication::tr("Ok"));
    messageBox.exec();
}

void GUI::_showError(const QString& title, const QString& msg) {
    QMessageBox messageBox(QMessageBox::Critical, title, msg, QMessageBox::Ok, getMainWidget());
    messageBox.setButtonText(QMessageBox::Ok, QApplication::tr("Ok"));
    messageBox.exec();
}

bool GUI::_askQuestion(const QString& title, const QString& msg, bool defaultAns, bool warning,
                       bool yesno) {
    QString positiveButton = yesno ? QApplication::tr("Yes") : QApplication::tr("Ok");
    QString negativeButton = yesno ? QApplication::tr("No") : QApplication::tr("Cancel");

    return _askQuestion(title, msg, positiveButton, negativeButton, defaultAns, warning);
}

bool GUI::_askQuestion(const QString& title, const QString& msg, const QString& button1,
                       const QString& button2, bool defaultAns, bool warning) {
    QMessageBox::Icon icon = warning ? QMessageBox::Warning : QMessageBox::Question;
    QMessageBox box(icon, title, msg, QMessageBox::NoButton, getMainWidget());
    QPushButton* pushButton1 = box.addButton(button1, QMessageBox::AcceptRole);
    QPushButton* pushButton2 = box.addButton(button2, QMessageBox::RejectRole);
    box.setDefaultButton(defaultAns ? pushButton1 : pushButton2);
    box.setEscapeButton(pushButton2);

    box.exec();
    return box.clickedButton() == pushButton1;
}

/**
 * @brief Get the main widget.
 * @return The main QWidget* of the application
 */
QWidget* GUI::getMainWidget() {
    return ok::Application::Instance()->getMainWidget();
}
}  // namespace lib::ui
