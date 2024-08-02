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

#include "src/widget/splitterrestorer.h"

#include <QSplitter>

/**
 * @class SplitterRestorer
 * @brief Restore splitter from saved state and reset to default
 */

/**
 * @brief The width of the default splitter handles.
 * By default, this property contains a value that depends on the user's
 * platform and style preferences.
 */
static int defaultWidth = 0;

/**
 * @brief Width of left splitter size in percents.
 */
const static int leftWidthPercent = 33;

SplitterRestorer::SplitterRestorer(QSplitter* splitter) : splitter{splitter} {
    if (defaultWidth == 0) {
        defaultWidth = QSplitter().handleWidth();
    }
}

/**
 * @brief Restore splitter from state. And reset in case of error.
 * Set the splitter to a reasonnable width by default and on first start
 * @param state State saved by QSplitter::saveState()
 * @param windowSize Widnow size (used to calculate splitter size)
 */
void SplitterRestorer::restore(const QByteArray& state, const QSize& windowSize) {
    bool brokenSplitter = !splitter->restoreState(state) ||
                          splitter->orientation() != Qt::Horizontal ||
                          splitter->handleWidth() > defaultWidth;

    if (splitter->count() == 2 && brokenSplitter) {
        splitter->setOrientation(Qt::Horizontal);
        splitter->setHandleWidth(defaultWidth);
        splitter->resize(windowSize);
        QList<int> sizes = splitter->sizes();
        sizes[0] = splitter->width() * leftWidthPercent / 100;
        sizes[1] = splitter->width() - sizes[0];
        splitter->setSizes(sizes);
    }
}
