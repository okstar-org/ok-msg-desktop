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

#ifndef CHATLOG_H
#define CHATLOG_H

#include <QDateTime>
#include <QGraphicsView>
#include <QMargins>

#include "chatline.h"
#include "chatmessage.h"
#include "src/lib/settings/style.h"

class QGraphicsScene;
class QGraphicsRectItem;
class QMouseEvent;
class QTimer;
class ChatLineContent;
struct ToxFile;

class ChatLog : public QGraphicsView {
    Q_OBJECT
public:
    explicit ChatLog(QWidget* parent = nullptr);
    virtual ~ChatLog();

    void insertChatlineAtBottom(IChatItem::Ptr l);
    void insertChatlineOnTop(IChatItem::Ptr l);
    void insertChatlinesOnTop(const QList<IChatItem::Ptr>& newLines);
    void clearSelection();
    void clear();
    void copySelectedText(bool toSelectionBuffer = false) const;
    void setBusyNotification(IChatItem::Ptr notification);
    void setTypingNotification(IChatItem::Ptr notification);
    void setTypingNotificationVisible(bool visible);
    void scrollToLine(IChatItem::Ptr line);
    void selectAll();
    void fontChanged(const QFont& font);
    void reloadTheme();

    QString getSelectedText() const;

    bool isEmpty() const;
    bool hasTextToBeCopied() const;

    IChatItem::Ptr getTypingNotification() const;
    QVector<IChatItem::Ptr> getLines();
    IChatItem::Ptr getLatestLine() const;
    IChatItem::Ptr getFirstLine() const;
    ChatLineContent* getContentFromGlobalPos(QPoint pos) const;
    const uint repNameAfter = 5 * 60;

    constexpr inline int getVScrollBarValue() const { return scrollBarValue; }
signals:
    void selectionChanged();
    void workerTimeoutFinished();
    void firstVisibleLineChanged(const IChatItem::Ptr& prevLine, const IChatItem::Ptr& firstLine);
    void loadHistoryLower();
    void readAll();

public slots:
    void forceRelayout();
private slots:
    void onSelectionTimerTimeout();
    void onWorkerTimeout();
    void onMultiClickTimeout();
    void onVScrollBarValueChanged(int value);

protected:
    QRectF calculateSceneRect() const;
    QRect getVisibleRect() const;
    ChatLineContent* getContentFromPos(QPointF scenePos) const;

    void layout(int start, int end, qreal width);
    bool isOverSelection(QPointF scenePos) const;
    bool stickToBottom() const;

    qreal useableWidth() const;

    void reposition(int start, int end, qreal deltaY);
    void updateSceneRect();
    void checkVisibility(bool causedByScroll = false);
    void scrollToBottom();
    void startResizeWorker();

    virtual void mouseDoubleClickEvent(QMouseEvent* ev) final override;
    virtual void mousePressEvent(QMouseEvent* ev) final override;
    virtual void mouseReleaseEvent(QMouseEvent* ev) final override;
    virtual void mouseMoveEvent(QMouseEvent* ev) final override;
    virtual void scrollContentsBy(int dx, int dy) final override;
    virtual void resizeEvent(QResizeEvent* ev) final override;
    virtual void showEvent(QShowEvent*) final override;
    virtual void focusInEvent(QFocusEvent* ev) final override;
    virtual void focusOutEvent(QFocusEvent* ev) final override;

    void updateMultiSelectionRect();
    void updateTypingNotification();
    void updateBusyNotification();

    IChatItem::Ptr findLineByPosY(qreal yPos) const;

private:
    void retranslateUi();
    bool isActiveFileTransfer(IChatItem::Ptr l);
    void handleMultiClickEvent();
    void moveSelectionRectUpIfSelected(int offset);
    void moveSelectionRectDownIfSelected(int offset);
    void movePreciseSelectionDown(int offset);
    void movePreciseSelectionUp(int offset);
    void moveMultiSelectionUp(int offset);
    void moveMultiSelectionDown(int offset);

private:
    enum class SelectionMode {
        None,
        Precise,
        Multi,
    };

    enum class AutoScrollDirection {
        NoDirection,
        Up,
        Down,
    };

    QAction* copyAction = nullptr;
    QAction* selectAllAction = nullptr;
    QGraphicsScene* scene = nullptr;
    QGraphicsScene* busyScene = nullptr;
    QVector<IChatItem::Ptr> lines;
    QList<IChatItem::Ptr> visibleLines;
    IChatItem::Ptr typingNotification;
    IChatItem::Ptr busyNotification;

    // selection
    int selClickedRow = -1;  // These 4 are only valid while selectionMode != None
    int selClickedCol = -1;
    int selFirstRow = -1;
    int selLastRow = -1;
    QColor selectionRectColor = Style::getColor(Style::SelectText);
    SelectionMode selectionMode = SelectionMode::None;
    QPointF clickPos;
    QGraphicsRectItem* selGraphItem = nullptr;
    QRectF selectionBox;
    QTimer* selectionTimer = nullptr;
    QTimer* workerTimer = nullptr;
    QTimer* multiClickTimer = nullptr;
    AutoScrollDirection selectionScrollDir = AutoScrollDirection::NoDirection;
    int clickCount = 0;
    QPoint lastClickPos;
    Qt::MouseButton lastClickButton;

    // worker vars
    int workerLastIndex = 0;
    bool workerStb = false;
    IChatItem::Ptr workerAnchorLine;

    // layout
    QMargins margins = QMargins(10, 10, 10, 10);
    qreal lineSpacing = 20.0f;

    int scrollBarValue = 0;
};

#endif  // CHATLOG_H
