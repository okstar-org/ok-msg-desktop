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

#ifndef SEARCHFORM_H
#define SEARCHFORM_H

#include <QLineEdit>
#include <QWidget>
#include "searchtypes.h"

class QPushButton;
class QLabel;
class SearchSettingsForm;
namespace SearchFormUI {
class LineEdit;
}

class SearchForm final : public QWidget {
    Q_OBJECT
public:
    enum class ToolButtonState {
        Disabled = 0,  // Grey
        Common = 1,    // Green
        Active = 2,    // Red
    };

    explicit SearchForm(QWidget* parent = nullptr);
    void removeSearchPhrase();
    QString getSearchPhrase() const;
    ParameterSearch getParameterSearch();
    void setFocusEditor();
    void insertEditor(const QString& text);
    void reloadTheme();

protected:
    virtual void showEvent(QShowEvent* event) final override;

private:
    // TODO: Merge with 'createButton' from chatformheader.cpp
    QPushButton* createButton(const QString& name, const QString& state);
    ParameterSearch getAndCheckParametrSearch();
    void setStateName(QPushButton* btn, ToolButtonState state);
    void useBeginState();

    QPushButton* settingsButton;
    QPushButton* upButton;
    QPushButton* downButton;
    QPushButton* hideButton;
    QPushButton* startButton;
    SearchFormUI::LineEdit* searchLine;
    SearchSettingsForm* settings;
    QLabel* messageLabel;

    QString searchPhrase;
    ParameterSearch parameter;

    bool isActiveSettings{false};
    bool isChangedPhrase{false};
    bool isSearchInBegin{true};
    bool isPrevSearch{false};

private slots:
    void changedSearchPhrase(const QString& text);
    void clickedUp();
    void clickedDown();
    void clickedHide();
    void clickedStart();
    void clickedSearch();
    void changedState(bool isUpdate);

public slots:
    void showMessageNotFound(SearchDirection direction);

signals:
    void searchInBegin(const QString& phrase, const ParameterSearch& parameter);
    void searchUp(const QString& phrase, const ParameterSearch& parameter);
    void searchDown(const QString& phrase, const ParameterSearch& parameter);
    void visibleChanged();
};

namespace SearchFormUI {
class LineEdit : public QLineEdit {
    Q_OBJECT

public:
    LineEdit(QWidget* parent = nullptr);

protected:
    virtual void keyPressEvent(QKeyEvent* event) final override;

signals:
    void clickEnter();
    void clickShiftEnter();
    void clickEsc();
};
}  // namespace SearchFormUI

#endif  // SEARCHFORM_H
