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
#include "launcher.h"

#include "base/r.h"
#include "application.h"
#include "base/system/sys_info.h"
#include <QTranslator>
#include <base/logs.h>
#include <memory>

namespace core {

std::unique_ptr<Launcher> Launcher::Create(int argc, char *argv[]) {
  return std::make_unique<Launcher>(argc, argv);
}

Launcher::Launcher(int argc, char *argv[])
    : _argc(argc), //
      _argv(argv)  //
{
  QThread::currentThread()->setObjectName("Launcher");
}

int Launcher::executeApplication() {

  base::CpuInfo cpuInfo;
  base::SysInfo::GetCpuInfo(cpuInfo);
  //"x64" "AuthenticAMD" "AMD Ryzen 7 1700X Eight-Core Processor" 8 16
  qDebug() << "CpuInfo:"          //
           << cpuInfo.arch         //
           << cpuInfo.manufacturer //
           << cpuInfo.name         //
           << cpuInfo.cores        //
           << cpuInfo.processors;  //

  base::OsInfo osInfo;
  base::SysInfo::GetOsInfo(osInfo);
  qDebug() << "OsInfo:"           //
           << osInfo.kernelName    //"linux"
           << osInfo.kernelVersion //"5.19.0-50-generic"
           << osInfo.name          //"ubuntu"
           << osInfo.version       //"22.04"
           << osInfo.prettyName    //"Ubuntu 22.04.2 LTS"
           << osInfo.hostName      //"root-host"
           << osInfo.uniqueId;     //"OWVjYjNmZTY0OTFmNGZiZGFhYjI0ODA2OTgwY2QxODQ="

  qDebug() <<"Release:" << APPLICATION_RELEASE << APPLICATION_VERSION_ID;

#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
  QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
  QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
  qputenv("QT_ENABLE_HIGHDPI_SCALING", "1");
  QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
      Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

  // 初始化程序实例
  app = new Application(_argc, _argv);

  // Windows platform plugins DLL hell fix
  app->addLibraryPath(QCoreApplication::applicationDirPath());
  app->addLibraryPath("platforms");
  app->start();
  app->finish();

  return app->exec();
}

int Launcher::startup() {
  auto result = executeApplication();
  qDebug() << QString("finished, result: %1").arg(result);
  return result;
}

int Launcher::shutdown() {
  app->closeAllWindows();
  return 0;
}

} // namespace core
