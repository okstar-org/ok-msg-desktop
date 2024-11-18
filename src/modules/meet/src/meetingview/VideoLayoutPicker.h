#pragma once

#include <QFrame>
#include "MeetingVideoDefines.h"

class QPushButton;
class QButtonGroup;
class QEventLoop;

class VideoLayoutPicker : public QFrame {
    Q_OBJECT
public:
    VideoLayoutPicker(QWidget* parent);
    void setCurrentType(module::meet::VideoLayoutType type);
    module::meet::VideoLayoutType selectedType() const;
    void exec(const QPoint & pos);

private:
    void onTypeChanged(int id, bool checked);

    void closeEvent(QCloseEvent* e);

private:
    QPushButton * appendItem(const QString& text, const QString& svgPath);

private:
    QButtonGroup* typeGroup = nullptr;
    QEventLoop* eventLoop = nullptr;
};