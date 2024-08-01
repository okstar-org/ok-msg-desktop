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

#ifndef __INTERFACES_H__
#define __INTERFACES_H__

G_BEGIN_DECLS

#define WATCHER_NAME "org.kde.StatusNotifierWatcher"
#define WATCHER_OBJECT "/StatusNotifierWatcher"
#define WATCHER_INTERFACE "org.kde.StatusNotifierWatcher"

#define ITEM_NAME "org.kde.StatusNotifierItem"
#define ITEM_OBJECT "/StatusNotifierItem"
#define ITEM_INTERFACE "org.kde.StatusNotifierItem"

static const gchar watcher_xml[] =
        "<node>"
        "   <interface name='org.kde.StatusNotifierWatcher'>"
        "       <property name='IsStatusNotifierHostRegistered' type='b' access='read' />"
        "       <method name='RegisterStatusNotifierItem'>"
        "           <arg name='service' type='s' direction='in' />"
        "       </method>"
        "       <signal name='StatusNotifierHostRegistered' />"
        "       <signal name='StatusNotifierHostUnregistered' />"
        "   </interface>"
        "</node>";

static const gchar item_xml[] =
        "<node>"
        "   <interface name='org.kde.StatusNotifierItem'>"
        "       <property name='Id' type='s' access='read' />"
        "       <property name='Category' type='s' access='read' />"
        "       <property name='Title' type='s' access='read' />"
        "       <property name='Status' type='s' access='read' />"
        "       <property name='WindowId' type='i' access='read' />"
        "       <property name='IconName' type='s' access='read' />"
        "       <property name='IconPixmap' type='(iiay)' access='read' />"
        "       <property name='OverlayIconName' type='s' access='read' />"
        "       <property name='OverlayIconPixmap' type='(iiay)' access='read' />"
        "       <property name='AttentionIconName' type='s' access='read' />"
        "       <property name='AttentionIconPixmap' type='(iiay)' access='read' />"
        "       <property name='AttentionMovieName' type='s' access='read' />"
        "       <property name='ToolTip' type='(s(iiay)ss)' access='read' />"
        "       <method name='ContextMenu'>"
        "           <arg name='x' type='i' direction='in' />"
        "           <arg name='y' type='i' direction='in' />"
        "       </method>"
        "       <method name='Activate'>"
        "           <arg name='x' type='i' direction='in' />"
        "           <arg name='y' type='i' direction='in' />"
        "       </method>"
        "       <method name='SecondaryActivate'>"
        "           <arg name='x' type='i' direction='in' />"
        "           <arg name='y' type='i' direction='in' />"
        "       </method>"
        "       <method name='Scroll'>"
        "           <arg name='delta' type='i' direction='in' />"
        "           <arg name='orientation' type='s' direction='in' />"
        "       </method>"
        "       <signal name='NewTitle' />"
        "       <signal name='NewIcon' />"
        "       <signal name='NewAttentionIcon' />"
        "       <signal name='NewOverlayIcon' />"
        "       <signal name='NewToolTip' />"
        "       <signal name='NewStatus'>"
        "           <arg name='status' type='s' />"
        "       </signal>"
        "   </interface>"
        "</node>";

G_END_DECLS

#endif /* __INTERFACES_H__ */
