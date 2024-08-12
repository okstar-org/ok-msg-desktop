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

#ifndef CROPPINGLABEL_H
#define CROPPINGLABEL_H

#include <QLabel>

class QLineEdit;

class CroppingLabel : public QLabel {
    Q_OBJECT
public:
    explicit CroppingLabel(QWidget* parent = nullptr);

public slots:
    void editBegin();
    void setEditable(bool editable);
    void setElideMode(Qt::TextElideMode elide);

    QString fullText();

public slots:
    void setText(const QString& text);
    void setPlaceholderText(const QString& text);
    void minimizeMaximumWidth();

signals:
    void editFinished(const QString& newText);
    void editRemoved();
    void clicked();

protected:
    void paintEvent(QPaintEvent* paintEvent) override;
    void setElidedText();
    void hideTextEdit();
    void showTextEdit();
    virtual void resizeEvent(QResizeEvent* ev) final override;
    virtual QSize sizeHint() const final override;
    virtual QSize minimumSizeHint() const final override;
    virtual void mouseReleaseEvent(QMouseEvent* e) final override;

private slots:
    void editingFinished();

private:
    QString origText;
    QLineEdit* textEdit;
    bool blockPaintEvents;
    bool editable;
    Qt::TextElideMode elideMode;
};

#endif  // CROPPINGLABEL_H
