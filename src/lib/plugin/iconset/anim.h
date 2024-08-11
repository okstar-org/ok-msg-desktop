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

#ifndef ANIM_H
#define ANIM_H

#include <QByteArray>
#include <QSharedDataPointer>

class Impix;
class QImage;
class QObject;
class QPixmap;
class QThread;

class Anim {
public:
    Anim();
    Anim(const QByteArray& data);
    Anim(const Anim& anim);
    ~Anim();

    const QPixmap& framePixmap() const;
    const QImage& frameImage() const;
    const Impix& frameImpix() const;
    bool isNull() const;

    int frameNumber() const;
    int numFrames() const;
    const Impix& frame(int n) const;

    bool paused() const;
    void unpause();
    void pause();

    void restart();

    void stripFirstFrame();

    static QThread* mainThread();
    static void setMainThread(QThread*);

    void connectUpdate(QObject* receiver, const char* member);
    void disconnectUpdate(QObject* receiver, const char* member = nullptr);

    Anim& operator=(const Anim&);
    Anim copy() const;
    void detach();

    class Private;

private:
    QSharedDataPointer<Private> d;
};

#endif  // ANIM_H
