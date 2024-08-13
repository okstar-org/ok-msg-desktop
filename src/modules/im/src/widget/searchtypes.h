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

#ifndef SEARCHTYPES_H
#define SEARCHTYPES_H

#include <QDate>
#include <QRegularExpression>

enum class FilterSearch {
    None,
    Register,
    WordsOnly,
    Regular,
    RegisterAndWordsOnly,
    RegisterAndRegular
};

enum class PeriodSearch { None, WithTheEnd, WithTheFirst, AfterDate, BeforeDate };

enum class SearchDirection { Up, Down };

struct ParameterSearch {
    FilterSearch filter{FilterSearch::None};
    PeriodSearch period{PeriodSearch::None};
    QDate date;
    bool isUpdate{false};

    bool operator==(const ParameterSearch& other) {
        return filter == other.filter && period == other.period && date == other.date;
    }

    bool operator!=(const ParameterSearch& other) { return !(*this == other); }
};

class SearchExtraFunctions {
public:
    /**
     * @brief generateFilterWordsOnly generate string for filter "Whole words only" for correct
     * search phrase containing symbols "\[]/^$.|?*+(){}"
     * @param phrase for search
     * @return new phrase for search
     */
    static QString generateFilterWordsOnly(const QString& phrase) {
        QString filter = QRegularExpression::escape(phrase);

        const QString symbols = QStringLiteral("\\[]/^$.|?*+(){}");

        if (filter != phrase) {
            if (filter.left(1) != QLatin1String("\\")) {
                filter = QLatin1String("\\b") + filter;
            } else {
                filter = QLatin1String("(^|\\s)") + filter;
            }
            if (!symbols.contains(filter.right(1))) {
                filter += QLatin1String("\\b");
            } else {
                filter += QLatin1String("($|\\s)");
            }
        } else {
            filter = QStringLiteral("\\b%1\\b").arg(filter);
        }

        return filter;
    }
};

#endif  // SEARCHTYPES_H
