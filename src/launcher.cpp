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

#include <memory>

#include "application.h"
#include "ipc.h"

namespace ok {

std::unique_ptr<Launcher> Launcher::Create(int argc, char* argv[]) {
    return std::make_unique<Launcher>(argc, argv);
}

Launcher::Launcher(int argc, char* argv[]) : _argc(argc), _argv(argv) {
    QThread::currentThread()->setObjectName("Launcher");
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "1");
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
            Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif
}

int Launcher::executeApplication() {

    app = new Application(_argc, _argv);

    // Windows platform plugins DLL
    app->addLibraryPath(QCoreApplication::applicationDirPath());
    app->addLibraryPath("platforms");
    app->start();
    app->finish();
    return app->exec();
}

int Launcher::startup() { return executeApplication(); }

void Launcher::shutdown() {
    app->closeAllWindows();
    qApp->exit(0);
}

}  // namespace ok
