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

#ifndef GUI_H
#define GUI_H

#include <QObject>

class QWidget;

class GUI : public QObject {
    Q_OBJECT
public:
    static GUI& getInstance();
    static QWidget* getMainWidget();
    static void setEnabled(bool state);
    static void setWindowTitle(const QString& title);
    static void reloadTheme();
    static void showInfo(const QString& title, const QString& msg);
    static void showWarning(const QString& title, const QString& msg);
    static void showError(const QString& title, const QString& msg);
    static bool askQuestion(const QString& title, const QString& msg, bool defaultAns = false,
                            bool warning = true, bool yesno = true);

    static bool askQuestion(const QString& title, const QString& msg, const QString& button1,
                            const QString& button2, bool defaultAns = false, bool warning = true);

signals:
    void themeApplyRequest();

private:
    explicit GUI(QObject* parent = nullptr);

private slots:
    // Private implementation, those must be called from the GUI thread
    void _setEnabled(bool state);
    void _setWindowTitle(const QString& title);
    void _reloadTheme();
    void _showInfo(const QString& title, const QString& msg);
    void _showWarning(const QString& title, const QString& msg);
    void _showError(const QString& title, const QString& msg);
    bool _askQuestion(const QString& title, const QString& msg, bool defaultAns = false,
                      bool warning = true, bool yesno = true);
    bool _askQuestion(const QString& title, const QString& msg, const QString& button1,
                      const QString& button2, bool defaultAns = false, bool warning = true);
};

#endif  // GUI_H
