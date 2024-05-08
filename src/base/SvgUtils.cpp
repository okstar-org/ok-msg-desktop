/*
 * Copyright (c) 2022 船山信息 chuanshaninfo.com
 * The project is licensed under Mulan PubL v2.
 * You can use this software according to the terms and conditions of the Mulan
 * PubL v2. You may obtain a copy of Mulan PubL v2 at:
 *          http://license.coscl.org.cn/MulanPubL-2.0
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PubL v2 for more details.
 */

//
// Created by gaojie on 24-5-8.
//

#include "SvgUtils.h"
#include <QIcon>
#include <QSvgRenderer>
#include <QPainter>


QIcon SvgUtils::prepareIcon(QString path, int w, int h) {
#ifdef Q_OS_LINUX
  // Preparing needed to set correct size of icons for GTK tray backend
  QString desktop = getenv("XDG_CURRENT_DESKTOP");
  if (desktop.isEmpty()) {
    desktop = getenv("DESKTOP_SESSION");
  }

  desktop = desktop.toLower();
  if (desktop == "xfce" || desktop.contains("gnome") || desktop == "mate" ||
      desktop == "x-cinnamon") {
    if (w > 0 && h > 0) {
      QSvgRenderer renderer(path);

      QPixmap pm(w, h);
      pm.fill(Qt::transparent);
      QPainter painter(&pm);
      renderer.render(&painter, pm.rect());

      return QIcon(pm);
    }
  }
#endif
  return QIcon(path);
}
