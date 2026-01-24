
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QMapLibre/Utils>
#include <QLoggingCategory>

#ifdef BUILD_HUSKARUI_STATIC_LIBRARY
#include <QtQml/qqmlextensionplugin.h>
Q_IMPORT_QML_PLUGIN(HuskarUI_BasicPlugin)
#endif

#include "customtheme.h"
#include "husapp.h"
#include "Application.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.addLibraryPath(app.applicationDirPath());

    // 创建并初始化 YEFS 应用
    YEFS::Application yefsApp(&app);
    
    if (!yefsApp.initialize()) {
        return -1;
    }

    // 注册自定义主题
    CustomTheme::instance()->registerAll();

    return yefsApp.run();
}
