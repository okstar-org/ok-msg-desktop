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

#ifndef VERTICALONLYSCROLLER_H
#define VERTICALONLYSCROLLER_H

#include <QScrollArea>

class QResizeEvent;
class QShowEvent;

namespace ok::base {

class VerticalOnlyScroller : public QScrollArea {
    Q_OBJECT
public:
    explicit VerticalOnlyScroller(QWidget* parent = nullptr);

protected:
    virtual void resizeEvent(QResizeEvent* event) final override;
    virtual void showEvent(QShowEvent* event) final override;
};
}  // namespace ok::base

#endif  // VERTICALONLYSCROLLER_H
