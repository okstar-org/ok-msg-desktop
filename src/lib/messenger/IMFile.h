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
#ifndef IMFILE_H
#define IMFILE_H

#include "lib/messenger/messenger.h"
#include <QFile>
#include <QThread>

namespace gloox{
class JID;
class BytestreamDataHandler;
}

namespace lib {
namespace messenger {

class IM;

//不要修改顺序和值
enum class FileStatus {
  INITIALIZING = 0,
  PAUSED = 1,
  TRANSMITTING = 2,
  BROKEN = 3,
  CANCELED = 4,
  FINISHED = 5,
};

//不要修改顺序和值
enum class FileDirection {
  SENDING = 0,
  RECEIVING = 1,
};

enum class FileControl{
  RESUME, PAUSE, CANCEL
};

struct File {
public:
  //id(file id = ibb id) 和 sId(session id)
  QString id;
  QString sId;
  QString name;
  QString path;
  quint64 size;
  FileStatus status;
  FileDirection direction;
  [[__nodiscard__ ]] QString toString() const;
  friend QDebug &operator<<(QDebug &debug, const File &f);
};

} // namespace messenger
} // namespace lib
#endif // OKEDU_CLASSROOM_DESKTOP_IMFILE_H
