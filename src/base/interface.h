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

#ifndef INTERFACE_H
#define INTERFACE_H

#include <QMetaObject>

#include <functional>

/**
 * @file interface.h
 *
 * Qt doesn't support QObject multiple inheritance. But for declaring signals
 * in class, it should be inherit QObject. To avoid this issue, interface can
 * provide some pure virtual methods, which allow to connect to some signal.
 *
 * This file provides macros to make signals declaring easly. With this macros
 * signal-like method will be declared and implemented in one line each.
 *
 * @example
 * class IExample {
 * public:
 *     // Like signal: void valueChanged(int value) const;
 *     // Declare `connectTo_valueChanged` method.
 *     DECLARE_SIGNAL(valueChanged, int value);
 * };
 *
 * class Example : public QObject, public IExample {
 * public:
 *     // Declare real signal and implement `connectTo_valueChanged`
 *     SIGNAL_IMPL(Example, valueChanged, int value);
 * };
 */
#define DECLARE_SIGNAL(name, ...)                         \
    using Slot_##name = std::function<void(__VA_ARGS__)>; \
    virtual QMetaObject::Connection connectTo_##name(QObject* receiver, Slot_##name slot) const = 0

/**
 * @def DECLARE_SIGNAL
 * @brief Declare signal-like method. Should be used in interface
 */
#define DECLARE_SIGNAL(name, ...)                         \
    using Slot_##name = std::function<void(__VA_ARGS__)>; \
    virtual QMetaObject::Connection connectTo_##name(QObject* receiver, Slot_##name slot) const = 0

/**
 * @def SIGNAL_IMPL
 * @brief Declare signal and implement signal-like method.
 */
#define SIGNAL_IMPL(classname, name, ...)                                                          \
    using Slot_##name = std::function<void(__VA_ARGS__)>;                                          \
    Q_SIGNAL void name(__VA_ARGS__);                                                               \
    QMetaObject::Connection connectTo_##name(QObject* receiver, Slot_##name slot) const override { \
        return connect(this, &classname::name, receiver, slot);                                    \
    }

#endif  // INTERFACE_H
