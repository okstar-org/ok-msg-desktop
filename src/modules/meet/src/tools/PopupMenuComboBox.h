#pragma once

#include <QFrame>
#include <QPointer>

class QToolButton;
class QHBoxLayout;
class QMenu;

class PopupMenuComboBox : public QFrame {
    Q_OBJECT

signals:
    void menuRequest();

public:
    PopupMenuComboBox(QWidget* parent = nullptr);
    void setLabel(const QString& text);
    void setWidget(QWidget* widget);
    QToolButton* iconButton();

    // 设置菜单
    void setMenu(QMenu * menu);
    // 手动显示自定义菜单
    void showMenuOnce(QMenu * menu);

private:
    void onMenuButtonClicked();

private:
    QHBoxLayout* mainLayout = nullptr;
    QToolButton* _iconButton = nullptr;
    QToolButton* menuButton = nullptr;
    QPointer<QWidget> content;
    QPointer<QMenu> popMenu;
};
