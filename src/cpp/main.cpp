
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QIcon>
#include <QMapLibre/Utils>

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

    return ret;
}
