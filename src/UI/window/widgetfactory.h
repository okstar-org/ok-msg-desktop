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
#ifndef WIDGETFACTORY_H
#define WIDGETFACTORY_H

#include <QObject>
#include <QWidget>
#include <list>

namespace UI {

class WidgetFactory : public QObject {
    Q_OBJECT

private:
    explicit WidgetFactory(QObject* parent = nullptr);

public:
    typedef std::list<QWidget*> WidgetList;

    template <typename T> static T* CreateWidget(QWidget* parent = nullptr) {
        //        static_assert(sizeof(QWidget) == sizeof(T),
        //                      "T type is not the QWidget");

        T* t = new T(parent);

        //        QPalette palette =  t->palette() ;
        //        t->setAutoFillBackground(true);
        //        palette.setColor(QPalette::Window, QColor("#838383"));
        //        palette.setColor(QPalette::Text, QColor("#ffffff"));
        //        t->setPalette(palette);

        //        widgets_.push_back(t);

        return t;
    }

private:
    //    WidgetList widgets_;

signals:

public slots:
};

}  // namespace UI
#endif  // WIDGETFACTORY_H
