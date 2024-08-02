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

#include "coloropt.h"

#include "OkOptions.h"

#include <QApplication>

using namespace ok::plugin;

ColorOpt::ColorOpt() : QObject(nullptr) {
    connect(OkOptions::instance(), SIGNAL(optionChanged(const QString&)),
            SLOT(optionChanged(const QString&)));
    connect(OkOptions::instance(), SIGNAL(destroyed()), SLOT(reset()));

    typedef struct {
        const char* opt;
        QPalette::ColorRole role;
    } SourceType;
    SourceType source[] = {{"contactlist.status.online", QPalette::Text},
                           {"contactlist.status.offline", QPalette::Text},
                           {"contactlist.status.away", QPalette::Text},
                           {"contactlist.status.do-not-disturb", QPalette::Text},
                           {"contactlist.profile.header-foreground", QPalette::Text},
                           {"contactlist.profile.header-background", QPalette::Dark},
                           {"contactlist.grouping.header-foreground", QPalette::Text},
                           {"contactlist.grouping.header-background", QPalette::Base},
                           {"contactlist.background", QPalette::Base},
                           {"contactlist.status-change-animation1", QPalette::Text},
                           {"contactlist.status-change-animation2", QPalette::Base},
                           {"contactlist.status-messages", QPalette::Text},
                           {"tooltip.background", QPalette::ToolTipBase},
                           {"tooltip.text", QPalette::ToolTipText},
                           {"messages.received", QPalette::Text},
                           {"messages.sent", QPalette::Text},
                           {"messages.informational", QPalette::Text},
                           {"messages.usertext", QPalette::Text},
                           {"messages.highlighting", QPalette::Text},
                           {"messages.link", QPalette::Link},
                           {"messages.link-visited", QPalette::Link},
                           {"passive-popup.border", QPalette::Window}};
    for (unsigned int i = 0; i < sizeof(source) / sizeof(SourceType); i++) {
        QString opt = QString("options.ui.look.colors.%1").arg(source[i].opt);
        colors.insert(opt, ColorData(OkOptions::instance()->getOption(opt).value<QColor>(),
                                     source[i].role));
    }
}

QColor ColorOpt::color(const QString& opt, const QColor& defaultColor) const {
    ColorData cd = colors.value(opt);
    // qDebug("get option: %s from data %s", qPrintable(opt), qPrintable(cd.color.isValid()?
    // cd.color.name() : "Invalid " + cd.color.name()));
    if (!cd.valid) {
        return OkOptions::instance()->getOption(opt, defaultColor).value<QColor>();
    }
    if (cd.color.isValid()) {
        return cd.color;
    }
    return QApplication::palette().color(cd.role);
}

QPalette::ColorRole ColorOpt::colorRole(const QString& opt) const { return colors.value(opt).role; }

void ColorOpt::optionChanged(const QString& opt) {
    if (opt.startsWith(QLatin1String("options.ui.look.colors")) && colors.contains(opt)) {
        colors[opt].color = OkOptions::instance()->getOption(opt).value<QColor>();
        // qDebug("%s changed to %s", qPrintable(opt), qPrintable(colors[opt].color.isValid()?
        // colors[opt].color.name() : "Invalid " + colors[opt].color.name()));
        emit changed(opt);
    }
}

/**
 * Returns the singleton instance of this class
 * \return Instance of OkOptions
 */
ColorOpt* ColorOpt::instance() {
    if (!instance_) instance_.reset(new ColorOpt());
    return instance_.data();
}

void ColorOpt::reset() { instance_.reset(nullptr); }

QScopedPointer<ColorOpt> ColorOpt::instance_;
