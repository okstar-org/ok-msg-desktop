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

#ifndef __STATUS_NOTIFIER_ENUMS_H__
#define __STATUS_NOTIFIER_ENUMS_H__
#include "statusnotifier.h"

GType status_notifier_error_get_type(void);
#define TYPE_STATUS_NOTIFIER_ERROR (status_notifier_error_get_type())
GType status_notifier_state_get_type(void);
#define TYPE_STATUS_NOTIFIER_STATE (status_notifier_state_get_type())
GType status_notifier_icon_get_type(void);
#define TYPE_STATUS_NOTIFIER_ICON (status_notifier_icon_get_type())
GType status_notifier_category_get_type(void);
#define TYPE_STATUS_NOTIFIER_CATEGORY (status_notifier_category_get_type())
GType status_notifier_status_get_type(void);
#define TYPE_STATUS_NOTIFIER_STATUS (status_notifier_status_get_type())
GType status_notifier_scroll_orientation_get_type(void);
#define TYPE_STATUS_NOTIFIER_SCROLL_ORIENTATION (status_notifier_scroll_orientation_get_type())
G_END_DECLS

#endif /* __STATUS_NOTIFIER_ENUMS_H__ */
