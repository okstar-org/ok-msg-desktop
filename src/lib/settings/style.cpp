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

#include "style.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFontInfo>
#include <QMap>
#include <QPainter>
#include <QRegularExpression>
#include <QSettings>
#include <QStandardPaths>
#include <QStringBuilder>
#include <QStyle>
#include <QWidget>

#include "OkSettings.h"

/**
 * @enum Style::Font
 *
 * @var ExtraBig
 * @brief [SystemDefault + 2]px, bold
 *
 * @var Big
 * @brief [SystemDefault]px
 *
 * @var BigBold
 * @brief [SystemDefault]px, bold
 *
 * @var Medium
 * @brief [SystemDefault - 1]px
 *
 * @var MediumBold
 * @brief [SystemDefault - 1]px, bold
 *
 * @var Small
 * @brief [SystemDefault - 2]px
 *
 * @var SmallLight
 * @brief [SystemDefault - 2]px, light
 *
 * @var BuiltinThemePath
 * @brief Path to the theme built into the application binary
 */

namespace {

const QLatin1String ThemeSubFolder{"themes/"};
const QLatin1String BuiltinThemeDefaultPath{":themes/default/"};
const QLatin1String BuiltinThemeDarkPath{":themes/dark/"};
}  // namespace

// helper functions
QFont appFont(int pixelSize, int weight) {
    QFont font;
    font.setPixelSize(pixelSize);
    font.setWeight(weight);
    return font;
}

QString qssifyFont(QFont font) {
    return QString("%1 %2px \"%3\"")
            .arg(font.weight() * 8)
            .arg(font.pixelSize())
            .arg(font.family());
}

namespace lib::settings {

static QMap<Style::ColorPalette, QColor> palette;
static QMap<QString, QColor> extPalette;

static QMap<QString, QString> dictColor;
static QMap<QString, QString> dictFont;
static QMap<QString, QString> dictTheme;
static QMap<QString, QString> dictExtColor;

QList<ThemeNameColor> Style::themeNameColors = {
        {MainTheme::Light, QObject::tr("Light"), QColor()},
        {MainTheme::Dark, QObject::tr("Dark"), QColor()},
};

QStringList Style::getThemeColorNames() {
    QStringList l;

    for (auto& t : themeNameColors) {
        l << t.name;
    }

    return l;
}

QString Style::getThemeName() {
    auto i = (int) OkSettings::getInstance().getThemeColor();
    if (i >=0 && i < themeNameColors.length())
        return themeNameColors[i].name;
    return themeNameColors[0].name;
}

QString Style::getThemeFolder() {
    const QString themeName = getThemeName();
    const QString themeFolder = ThemeSubFolder % themeName;
    const QString fullPath = QStandardPaths::locate(QStandardPaths::AppDataLocation, themeFolder,
                                                    QStandardPaths::LocateDirectory);

    // No themes available, fallback to builtin
    if (fullPath.isEmpty()) {
        return getThemePath();
    }

    return fullPath % QDir::separator();
}

QMap<Style::ColorPalette, QString> Style::aliasColors = {{ColorPalette::TransferGood, "transferGood"},
                                                         {ColorPalette::TransferWait, "transferWait"},
                                                         {ColorPalette::TransferBad, "transferBad"},
                                                         {ColorPalette::TransferMiddle, "transferMiddle"},
                                                         {ColorPalette::MainText, "mainText"},
                                                         {ColorPalette::NameActive, "nameActive"},
                                                         {ColorPalette::StatusActive, "statusActive"},
                                                         {ColorPalette::GroundExtra, "groundExtra"},
                                                         {ColorPalette::GroundBase, "groundBase"},
                                                         {ColorPalette::Orange, "orange"},
                                                         {ColorPalette::ThemeDark, "themeDark"},
                                                         {ColorPalette::ThemeMediumDark, "themeMediumDark"},
                                                         {ColorPalette::ThemeMedium, "themeMedium"},
                                                         {ColorPalette::ThemeLight, "themeLight"},
                                                         {ColorPalette::ThemeHighlight, "themeHighlight"},
                                                         {ColorPalette::Action, "action"},
                                                         {ColorPalette::Link, "link"},
                                                         {ColorPalette::SearchHighlighted, "searchHighlighted"},
                                                         {ColorPalette::SelectText, "selectText"}};

// stylesheet filename, font -> stylesheet
// QString implicit sharing deduplicates stylesheets rather than constructing a new one each time
std::map<std::pair<const QString, const QFont>, const QString> Style::stylesheetsCache;

const QString Style::getStylesheet(const QString& filename, const QFont& baseFont) {
    QString folder = QDir::isAbsolutePath(filename) ? QString() : getThemeFolder();
    const QString fullPath = folder + filename;
    //    qDebug() << "theme:" << fullPath;
    const std::pair<const QString, const QFont> cacheKey(fullPath, baseFont);
    auto it = stylesheetsCache.find(cacheKey);
    if (it != stylesheetsCache.end()) {
        // cache hit
        return it->second;
    }
    // cache miss, new styleSheet, read it from file and add to cache
    const QString newStylesheet = resolve(filename, baseFont);
    stylesheetsCache.insert(std::make_pair(cacheKey, newStylesheet));
    return newStylesheet;
}

static QStringList existingImagesCache;
const QString Style::getImagePath(const QString& filename) {
    QString fullPath = getThemeFolder() + filename;

    // search for image in cache
    if (existingImagesCache.contains(fullPath)) {
        return fullPath;
    }

    // if not in cache
    if (QFileInfo::exists(fullPath)) {
        existingImagesCache << fullPath;
        return fullPath;
    } else {
        qWarning() << "Failed to open file (using defaults):" << fullPath;

        fullPath = getThemePath() % filename;
        if (QFileInfo::exists(fullPath)) {
            return fullPath;
        } else {
            qWarning() << "Failed to open default file:" << fullPath;
            return {};
        }
    }
}

QColor Style::getColor(Style::ColorPalette entry) {
    return palette[entry];
}

QColor Style::getExtColor(const QString& key) {
    return extPalette.value(key, QColor(0, 0, 0));
}

QFont Style::getFont(Style::Font font) {
    // fonts as defined in
    // https://github.com/ItsDuke/Tox-UI/blob/master/UI%20GUIDELINES.md

    static int defSize = QFontInfo(QFont()).pixelSize();

    static QFont fonts[] = {
            appFont(defSize + 3, QFont::Bold),    // extra big
            appFont(defSize + 1, QFont::Normal),  // big
            appFont(defSize + 1, QFont::Bold),    // big bold
            appFont(defSize, QFont::Normal),      // medium
            appFont(defSize, QFont::Bold),        // medium bold
            appFont(defSize - 1, QFont::Normal),  // small
            appFont(defSize - 1, QFont::Light),   // small light
    };

    return fonts[(int)font];
}

const QString Style::resolve(const QString& filename, const QFont& baseFont) {
    QString themePath = QDir::isAbsolutePath(filename) ? QString() : getThemeFolder();
    QString fullPath = themePath + filename;
    QString qss;

    QFile file{fullPath};
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        qss = file.readAll();
    } else {
        qWarning() << "Failed to open file (using defaults):" << fullPath;

        fullPath = getThemePath();
        QFile file{fullPath};

        if (file.open(QFile::ReadOnly | QFile::Text)) {
            qss = file.readAll();
        } else {
            qWarning() << "Failed to open default file:" << fullPath;
            return {};
        }
    }

    if (palette.isEmpty()) {
        initPalette();
    }

    if (dictColor.isEmpty()) {
        initDictColor();
    }

    if (dictFont.isEmpty()) {
        dictFont = {{"@baseFont", QString::fromUtf8("'%1' %2px")
                                          .arg(baseFont.family())
                                          .arg(QFontInfo(baseFont).pixelSize())},
                    {"@extraBig", qssifyFont(Style::getFont(Style::Font::ExtraBig))},
                    {"@big", qssifyFont(Style::getFont(Style::Font::Big))},
                    {"@bigBold", qssifyFont(Style::getFont(Style::Font::BigBold))},
                    {"@medium", qssifyFont(Style::getFont(Style::Font::Medium))},
                    {"@mediumBold", qssifyFont(Style::getFont(Style::Font::MediumBold))},
                    {"@small", qssifyFont(Style::getFont(Style::Font::Small))},
                    {"@smallLight", qssifyFont(Style::getFont(Style::Font::SmallLight))}};
    }

    if (dictExtColor.isEmpty() && !extPalette.isEmpty()) {
        auto it = extPalette.begin();
        while (it != extPalette.end()) {
            dictExtColor.insert("@" + it.key(), it.value().name());
            it++;
        }
    }

    QRegularExpression anchorReg(R"(@([a-zA-z0-9\.]+))");
    int from = 0;
    int index = qss.indexOf('@');
    while (index >= 0) {
        QRegularExpressionMatch match = anchorReg.match(qss, from);
        if (match.hasMatch()) {
            QString key = match.captured(0);
            // c++17
            if (auto it = dictColor.find(key); it != dictColor.end())
                qss.replace(key, it.value());
            else if (auto it = dictFont.find(key); it != dictFont.end())
                qss.replace(key, it.value());
            else if (auto it = dictTheme.find(key); it != dictTheme.end())
                qss.replace(key, it.value());
            else if (auto it = dictExtColor.find(key); it != dictExtColor.end())
                qss.replace(key, it.value());
        }
        from++;
        index = qss.indexOf('@', from);
    }

    // @getImagePath() function
    const QRegularExpression re{QStringLiteral(R"(@getImagePath\([^)\s]*\))")};
    QRegularExpressionMatchIterator i = re.globalMatch(qss);

    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString path = match.captured(0);
        const QString phrase = path;

        path.remove(QStringLiteral("@getImagePath("));
        path.chop(1);

        QString fullImagePath = getThemeFolder() + path;
        // image not in cache
        if (!existingImagesCache.contains(fullPath)) {
            if (QFileInfo::exists(fullImagePath)) {
                existingImagesCache << fullImagePath;
            } else {
                qWarning() << "Failed to open file (using defaults):" << fullImagePath;
                fullImagePath = getThemePath() % path;
            }
        }

        qss.replace(phrase, fullImagePath);
    }

    return qss;
}

void Style::repolish(QWidget* w) {
    w->style()->unpolish(w);
    w->style()->polish(w);

    for (QObject* o : w->children()) {
        QWidget* c = qobject_cast<QWidget*>(o);
        if (c) {
            c->style()->unpolish(c);
            c->style()->polish(c);
        }
    }
}

void Style::setThemeColor(int color) {
    stylesheetsCache.clear();  // clear stylesheet cache which includes color info
    palette.clear();
    dictColor.clear();
    dictExtColor.clear();
    initPalette();
    initDictColor();
    if (color < 0 || color >= themeNameColors.size())
        setThemeColor(QColor());
    else
        setThemeColor(themeNameColors[color].color);
}

/**
 * @brief Set theme color.
 * @param color Color to set.
 *
 * Pass an invalid QColor to reset to defaults.
 */
void Style::setThemeColor(const QColor& color) {
    if (!color.isValid()) {
        // Reset to default
        palette[ColorPalette::ThemeDark] = getColor(ColorPalette::ThemeDark);
        palette[ColorPalette::ThemeMediumDark] = getColor(ColorPalette::ThemeMediumDark);
        palette[ColorPalette::ThemeMedium] = getColor(ColorPalette::ThemeMedium);
        palette[ColorPalette::ThemeLight] = getColor(ColorPalette::ThemeLight);
    } else {
        palette[ColorPalette::ThemeDark] = color.darker(155);
        palette[ColorPalette::ThemeMediumDark] = color.darker(135);
        palette[ColorPalette::ThemeMedium] = color.darker(120);
        palette[ColorPalette::ThemeLight] = color.lighter(110);
    }

    dictTheme["@themeDark"] = getColor(ColorPalette::ThemeDark).name();
    dictTheme["@themeMediumDark"] = getColor(ColorPalette::ThemeMediumDark).name();
    dictTheme["@themeMedium"] = getColor(ColorPalette::ThemeMedium).name();
    dictTheme["@themeLight"] = getColor(ColorPalette::ThemeLight).name();
}

/**
 * @brief Reloads some CCS
 */
void Style::applyTheme() {
    //    GUI::reloadTheme();
}

void Style::initPalette() {
    QSettings settings(getThemePath() % "palette.ini", QSettings::IniFormat);

    settings.beginGroup("colors");
    QMap<Style::ColorPalette, QString> c;
    auto keys = aliasColors.keys();
    for (auto k : keys) {
        c[k] = settings.value(aliasColors[k], "#000").toString();
        palette[k] = QColor(settings.value(aliasColors[k], "#000").toString());
    }
    auto p = palette;
    settings.endGroup();

    settings.beginGroup("extends-colors");
    for (auto k : settings.childKeys()) {
        QColor color(settings.value(k).toString());
        if (color.isValid()) extPalette.insert(k, color);
    }
    settings.endGroup();
}

void Style::initDictColor() {
    dictColor = {{"@transferGood", Style::getColor(Style::ColorPalette::TransferGood).name()},
                 {"@transferWait", Style::getColor(Style::ColorPalette::TransferWait).name()},
                 {"@transferBad", Style::getColor(Style::ColorPalette::TransferBad).name()},
                 {"@transferMiddle", Style::getColor(Style::ColorPalette::TransferMiddle).name()},
                 {"@mainText", Style::getColor(Style::ColorPalette::MainText).name()},
                 {"@nameActive", Style::getColor(Style::ColorPalette::NameActive).name()},
                 {"@statusActive", Style::getColor(Style::ColorPalette::StatusActive).name()},
                 {"@groundExtra", Style::getColor(Style::ColorPalette::GroundExtra).name()},
                 {"@groundBase", Style::getColor(Style::ColorPalette::GroundBase).name()},
                 {"@orange", Style::getColor(Style::ColorPalette::Orange).name()},
                 {"@action", Style::getColor(Style::ColorPalette::Action).name()},
                 {"@link", Style::getColor(Style::ColorPalette::Link).name()},
                 {"@searchHighlighted", Style::getColor(Style::ColorPalette::SearchHighlighted).name()},
                 {"@selectText", Style::getColor(Style::ColorPalette::SelectText).name()}};
}

QString Style::getThemePath() {
    auto num = (int)OkSettings::getInstance().getThemeColor();
    if (themeNameColors[num].type == MainTheme::Dark) {
        return BuiltinThemeDarkPath;
    }
    return BuiltinThemeDefaultPath;
}

}  // namespace lib::settings
