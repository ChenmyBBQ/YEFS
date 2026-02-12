
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

// 过滤平台插件产生的无害警告，减少日志噪音
static QtMessageHandler s_defaultHandler = nullptr;
static void messageFilter(QtMsgType type, const QMessageLogContext &ctx, const QString &msg)
{
    // 过滤 Linux 平台不支持窗口透明度的警告
    if (type == QtWarningMsg && msg.contains(QStringLiteral("does not support setting window opacity")))
        return;
    if (s_defaultHandler)
        s_defaultHandler(type, ctx, msg);
}

int main(int argc, char *argv[])
{
    s_defaultHandler = qInstallMessageHandler(messageFilter);

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
