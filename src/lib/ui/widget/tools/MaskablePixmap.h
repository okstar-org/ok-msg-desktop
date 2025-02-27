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

#ifndef MASKABLEPIXMAP_H
#define MASKABLEPIXMAP_H

#include <QLabel>
namespace lib::ui {
class MaskablePixmapWidget : public QLabel {
    Q_OBJECT
public:
    MaskablePixmapWidget(QWidget* parent, QSize size, QString maskName = QString());
    ~MaskablePixmapWidget() override;
    void autopickBackground();
    void setClickable(bool clickable);
    void setPixmap(const QPixmap& pmap);
    QPixmap getPixmap() const;
    void setSize(QSize size);

protected:
    void mousePressEvent(QMouseEvent*) final override;

private:
    void updatePixmap();

    QPixmap pixmap, mask, unscaled;
    QPixmap* renderTarget;
    QString maskName;
    bool clickable;

signals:
    void clicked();
};
}  // namespace lib::ui
#endif  // MASKABLEPIXMAP_H
