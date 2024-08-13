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

#ifndef STYLE_H
#define STYLE_H

#include <QColor>
#include <QFont>

class QString;
class QWidget;

class Style {
public:
    enum ColorPalette {
        TransferGood,
        TransferWait,
        TransferBad,
        TransferMiddle,
        MainText,
        NameActive,
        StatusActive,
        GroundExtra,
        GroundBase,
        Orange,
        ThemeDark,
        ThemeMediumDark,
        ThemeMedium,
        ThemeLight,
        ThemeHighlight,
        Action,
        Link,
        SearchHighlighted,
        SelectText
    };

    enum Font { ExtraBig, Big, BigBold, Medium, MediumBold, Small, SmallLight };

    enum MainTheme { Light, Dark };

    struct ThemeNameColor {
        MainTheme type;
        QString name;
        QColor color;
    };

    static QStringList getThemeColorNames();
    static const QString getStylesheet(const QString& filename, const QFont& baseFont = QFont());
    static const QString getImagePath(const QString& filename);
    static QString getThemeFolder();
    static QString getThemeName();
    static QColor getColor(ColorPalette entry);
    static QColor getExtColor(const QString& key);
    static QFont getFont(Font font);
    static const QString resolve(const QString& filename, const QFont& baseFont = QFont());
    static void repolish(QWidget* w);
    static void setThemeColor(int color);
    static void setThemeColor(const QColor& color);
    static void applyTheme();
    static void initPalette();
    static void initDictColor();
    static QString getThemePath();

signals:
    void themeChanged();

private:
    Style();

private:
    static QList<ThemeNameColor> themeNameColors;
    static std::map<std::pair<const QString, const QFont>, const QString> stylesheetsCache;
    static QMap<ColorPalette, QString> aliasColors;
};

#endif  // STYLE_H
