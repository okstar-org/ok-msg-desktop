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

#ifndef PAINTERMDIAREA_H
#define PAINTERMDIAREA_H

#include <QMdiArea>
#include <memory>

namespace module::painter {

class CSharedPainterScene;

class PainterMdiArea : public QMdiArea {
    Q_OBJECT

public:
    explicit PainterMdiArea(std::shared_ptr<CSharedPainterScene> scene,
                            QWidget* parent = Q_NULLPTR);

    virtual ~PainterMdiArea() override;

protected:
    virtual void mouseReleaseEvent(QMouseEvent* e) override;
    virtual void mouseMoveEvent(QMouseEvent* e) override;
    virtual void mousePressEvent(QMouseEvent* e) override;

    virtual void keyPressEvent(QKeyEvent* e) override;

    virtual void dragEnterEvent(QDragEnterEvent* e) override;
    virtual void dragLeaveEvent(QDragLeaveEvent* e) override;
    virtual void dragMoveEvent(QDragMoveEvent* e) override;
    virtual void dropEvent(QDropEvent* e) override;

    bool eventFilter(QObject* watcher, QEvent* event) override;

private:
    virtual void forwardEvent(QMouseEvent* e);
    virtual void forwardEvent(QKeyEvent* e);

    virtual void forwardEvent(QDragEnterEvent* e);
    virtual void forwardEvent(QDragLeaveEvent* e);
    virtual void forwardEvent(QDragMoveEvent* e);
    virtual void forwardEvent(QDropEvent* e);

    std::shared_ptr<CSharedPainterScene> scene_;

public slots:
    void onDragLeaveEvent();
    void onDropEvent();
};
}  // namespace module::painter

#endif  // PAINTERMDIAREA_H
