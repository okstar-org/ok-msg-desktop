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

#include <QColor>
#include <QHash>
#include <QObject>
#include <QPalette>

class ColorData {
public:
    ColorData() : role(QPalette::NoRole), valid(false) {}
    ColorData(const QColor& color, QPalette::ColorRole role)
            : color(color), role(role), valid(true) {}

    QColor color;
    QPalette::ColorRole role;
    bool valid;
};

class ColorOpt : public QObject {
    Q_OBJECT
public:
    static ColorOpt* instance();
    QColor color(const QString& opt, const QColor& defaultColor = QColor()) const;
    QPalette::ColorRole colorRole(const QString& opt) const;

signals:
    void changed(const QString& opt);

private:
    ColorOpt();

public slots:
    static void reset();

private slots:
    void optionChanged(const QString& opt);

private:
    static QScopedPointer<ColorOpt> instance_;
    QHash<QString, ColorData> colors;
};
