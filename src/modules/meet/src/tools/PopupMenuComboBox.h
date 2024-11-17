#pragma once

#include <QFrame>
#include <QPointer>

class QToolButton;
class QHBoxLayout;

class PopupMenuComboBox : public QFrame {
    Q_OBJECT

signals:
    void menuRequest();

public:
    PopupMenuComboBox(QWidget* parent = nullptr);
    void setLabel(const QString& text);
    void setWidget(QWidget* widget);
    QToolButton* iconButton();

private:
    QHBoxLayout* mainLayout = nullptr;
    QToolButton* _iconButton = nullptr;
    QToolButton* menuButton = nullptr;
    QPointer<QWidget> content = nullptr;
};
