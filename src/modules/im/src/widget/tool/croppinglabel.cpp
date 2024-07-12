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

#include "croppinglabel.h"
#include <QApplication>
#include <QKeyEvent>
#include <QLineEdit>
#include <QResizeEvent>
#include <QStyle>
#include <QStyleOptionFrame>
#include <QTextDocument>

class LineEdit : public QLineEdit {
public:
  explicit LineEdit(QWidget *parent = nullptr) : QLineEdit(parent) {}

protected:
  void keyPressEvent(QKeyEvent *event) override {
    if (event->key() == Qt::Key_Escape) {
      undo();
      clearFocus();
    }

    QLineEdit::keyPressEvent(event);
  }
};

CroppingLabel::CroppingLabel(QWidget *parent) : QLabel(parent),
                                                blockPaintEvents(false),
                                                editable(false),
                                                elideMode(Qt::ElideRight) //
{
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  textEdit = new LineEdit(this);
  textEdit->hide();
  textEdit->setInputMethodHints(Qt::ImhNoAutoUppercase | Qt::ImhNoPredictiveText | Qt::ImhPreferLatin);

  connect(textEdit, &QLineEdit::editingFinished, this, &CroppingLabel::editingFinished);
}

void CroppingLabel::editBegin() {
  showTextEdit();
  textEdit->selectAll();
}

void CroppingLabel::setEditable(bool editable) {
  this->editable = editable;

  if (editable)
    setCursor(Qt::PointingHandCursor);
  else
    unsetCursor();
}

void CroppingLabel::setElideMode(Qt::TextElideMode elide) { elideMode = elide; }

void CroppingLabel::setText(const QString &text) {
  origText = text.trimmed();
  setElidedText();
}

void CroppingLabel::setPlaceholderText(const QString &text) {
  textEdit->setPlaceholderText(text);
  setElidedText();
}

void CroppingLabel::resizeEvent(QResizeEvent *ev) {
  setElidedText();
  textEdit->resize(ev->size());

  QLabel::resizeEvent(ev);
}

QSize CroppingLabel::sizeHint() const { return QSize(0, QLabel::sizeHint().height()); }

QSize CroppingLabel::minimumSizeHint() const {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
  QSize s(fontMetrics().horizontalAdvance("..."), QLabel::minimumSizeHint().height());
#else
  QSize s(fontMetrics().width("..."), QLabel::minimumSizeHint().height());
#endif
  const int v_margin = 2; // from Qt
  const int h_margin = 4; // from Qt
  s += QSize(h_margin, v_margin);
  QStyleOptionFrame opt;
  initStyleOption(&opt);
  return (style()->sizeFromContents(QStyle::CT_LineEdit, &opt, s.expandedTo(QApplication::globalStrut()), this));
}

void CroppingLabel::mouseReleaseEvent(QMouseEvent *e) {
  if (editable)
    showTextEdit();

  emit clicked();

  QLabel::mouseReleaseEvent(e);
}

void CroppingLabel::paintEvent(QPaintEvent *paintEvent) {
  if (blockPaintEvents) {
    paintEvent->ignore();
    return;
  }
  QLabel::paintEvent(paintEvent);
}

void CroppingLabel::setElidedText() {
  QString elidedText = fontMetrics().elidedText(origText, elideMode, width());
  if (elidedText != origText)
    setToolTip(Qt::convertFromPlainText(origText, Qt::WhiteSpaceNormal));
  else
    setToolTip(QString());
  if (!elidedText.isEmpty()) {
    QLabel::setText(elidedText);
  } else {
    // NOTE: it would be nice if the label had custom styling when it was default
    QLabel::setText(textEdit->placeholderText());
  }
}

void CroppingLabel::hideTextEdit() {
  textEdit->hide();
  blockPaintEvents = false;
}

void CroppingLabel::showTextEdit() {
  blockPaintEvents = true;
  textEdit->show();
  textEdit->setFocus();
  textEdit->setText(origText);
  textEdit->setFocusPolicy(Qt::ClickFocus);
}

/**
 * @brief Get original full text.
 * @return The un-cropped text.
 */
QString CroppingLabel::fullText() { return origText; }

void CroppingLabel::minimizeMaximumWidth() {
  // This function chooses the smallest possible maximum width.
  // Text width + padding. Without padding, we'll have elipses.
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
  setMaximumWidth(fontMetrics().horizontalAdvance(origText) + fontMetrics().horizontalAdvance("..."));
#else
  setMaximumWidth(fontMetrics().width(origText) + fontMetrics().width("..."));
#endif
}

void CroppingLabel::editingFinished() {
  hideTextEdit();
  QString newText = textEdit->text().trimmed().remove(QRegExp("[\\t\\n\\v\\f\\r\\x0000]"));

  if (origText != newText)
    emit editFinished(textEdit->text());

  emit editRemoved();
}
