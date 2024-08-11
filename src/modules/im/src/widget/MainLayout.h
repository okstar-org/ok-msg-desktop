#ifndef MAINLAYOUT_H
#define MAINLAYOUT_H

#include <QWidget>

#include "src/widget/contentlayout.h"

class MainLayout : public QWidget {
    Q_OBJECT
public:
    explicit MainLayout(QWidget* parent = nullptr);

    [[nodiscard]] virtual ContentLayout* getContentLayout() const = 0;

signals:
};

#endif  // MAINLAYOUT_H
