/*
 * Copyright (c) 2022 船山信息 chuanshaninfo.com
 * The project is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PubL v2 for more details.
 */

//
// Created by gaojie on 24-5-8.
//

#ifndef OKMSG_SVGUTILS_H
#define OKMSG_SVGUTILS_H

#include <QIcon>
#include <QPainter>
#include <QPixmap>
#include <QSvgRenderer>
namespace ok::base {
class SvgUtils {
public:
    /**
     * 将SVG渲染到QPixmap以支持不失真的伸缩
     * @brief renderTo
     * @param path
     * @param pm
     */
    inline static void renderTo(const QString& path, QPixmap& pm) {
        QSvgRenderer renderer(path);
        pm.fill(Qt::transparent);
        QPainter painter(&pm);
        renderer.render(&painter, pm.rect());
    }

    inline static QIcon prepareIcon(const QString& path, int w, int h) {
        if (!(w > 0 && h > 0)) {
            return QIcon{path};
        }

#ifdef Q_OS_LINUX
        QPixmap pm(w, h);
        renderTo(path, pm);
        return QIcon{pm};
#else
        return QIcon{path};
#endif
    }

    static QPixmap scaleSvgImage(const QString& path, quint32 width, quint32 height) {
        QSvgRenderer render(path);
        QPixmap pixmap(width, height);
        pixmap.fill(QColor(0, 0, 0, 0));
        QPainter painter(&pixmap);
        render.render(&painter, pixmap.rect());
        return pixmap;
    }
};
}
#endif  // OKMSG_PROJECT_SVGUTILS_H
