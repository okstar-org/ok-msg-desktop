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

#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QHBoxLayout>
#include <QPushButton>
#include <QStyleFactory>

#include <array>
#include <memory>

class Camera;
class GenericForm;
class GeneralForm;
class IAudioControl;
class PrivacyForm;
class AVForm;
class QLabel;
class QTabWidget;
class ContentLayout;
class Widget;

class SettingsWidget : public QWidget {
    Q_OBJECT
public:
    SettingsWidget(Widget* parent = nullptr);
    ~SettingsWidget();

    bool isShown() const;
    void show(ContentLayout* contentLayout);
    void setBodyHeadStyle(QString style);

    void showAbout();

public slots:
    void onUpdateAvailable(void);

private slots:
    void onTabChanged(int);

private:
    void retranslateUi();

private:
    std::unique_ptr<QVBoxLayout> bodyLayout;
    std::unique_ptr<QTabWidget> settingsWidgets;
    std::vector<std::unique_ptr<GenericForm>> cfgForms;
    int currentIndex;
};

#endif  // SETTINGSWIDGET_H
