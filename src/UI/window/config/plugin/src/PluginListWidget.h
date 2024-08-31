#ifndef PLUGINLISTWIDGET_H
#define PLUGINLISTWIDGET_H

#include <QListWidget>

class PluginListWidget : public QListWidget
{
    Q_OBJECT
public:
    PluginListWidget(QWidget* parent = nullptr);
};

#endif  // !PLUGINLISTWIDGET_H
