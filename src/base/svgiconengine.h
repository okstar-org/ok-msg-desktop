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

#ifndef SVGICONENGINE_H
#define SVGICONENGINE_H

#include <QIconEngine>
#include <QPixmapCache>
#include <QSvgRenderer>
#include <memory>
namespace ok::base {
class SvgIconEngine : public QIconEngine {
    QString name;
    std::shared_ptr<QSvgRenderer> renderer;

    struct CacheEntry {
        QPixmapCache::Key key;
        QSize size;
    };
    std::list<CacheEntry> normalCache, disabledCache;

public:
    SvgIconEngine(const QString& name, std::shared_ptr<QSvgRenderer> renderer)
            : name(name), renderer(renderer) {}

    QSize actualSize(const QSize& size, QIcon::Mode mode, QIcon::State state) override;
    QIconEngine* clone() const override;
    void paint(QPainter* painter, const QRect& rect, QIcon::Mode mode, QIcon::State state) override;
    void virtual_hook(int id, void* data) override;

    QPixmap pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state) override;

private:
    QPixmap renderPixmap(const QSize& size, QIcon::Mode mode, QIcon::State state);
};
}
#endif  // SVGICONENGINE_H
