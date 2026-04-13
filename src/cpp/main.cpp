
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QIcon>
#include <QMapLibre/Utils>

#ifdef Q_OS_WIN
#include <windows.h>
#else
#include <cstdlib>
#endif

#ifdef BUILD_HUSKARUI_STATIC_LIBRARY
#include <QtQml/qqmlextensionplugin.h>
Q_IMPORT_QML_PLUGIN(HuskarUI_BasicPlugin)
#endif

#include "customtheme.h"
#include "husapp.h"
#include "Application.h"
#include "logsetup.h"

int main(int argc, char *argv[])
{
    YEFS::LogSetup::initialize();

    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(QStringLiteral(":/YEFSApp/resources/images/YEFS.ico")));
    app.addLibraryPath(app.applicationDirPath());

    YEFS::Application yefsApp(&app);

    if (!yefsApp.initialize()) {
        return -1;
    }

    // 注册自定义主题
    CustomTheme::instance()->registerAll();

    int ret = yefsApp.run();
    qDebug() << "[main] yefsApp.run() returned:" << ret;

    // MapLibre 的 ThreadPool（std::thread 线程池）在 DLL 卸载阶段（DllMain）会发起 join 等待，
    // 而此时 Qt 事件循环和某些基础资源已销毁，跨线程互锁极易导致 ExitProcess 甚至整体进程死锁挂起。
    // 为了确保应用彻底无阻塞关闭，改用 TerminateProcess 绕过 DllMain 的卸载过程。
    YEFS::LogSetup::shutdown();
#ifdef Q_OS_WIN
    TerminateProcess(GetCurrentProcess(), static_cast<UINT>(ret));
#else
    std::_Exit(ret);
#endif
    return ret; // 消除 -Wreturn-type 警告
}
