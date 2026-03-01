
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QIcon>
#include <QMapLibre/Utils>
#include <QLoggingCategory>
#include <QFile>
#include <QDateTime>

#ifdef BUILD_HUSKARUI_STATIC_LIBRARY
#include <QtQml/qqmlextensionplugin.h>
Q_IMPORT_QML_PLUGIN(HuskarUI_BasicPlugin)
#endif

#include "customtheme.h"
#include "husapp.h"
#include "Application.h"

// 日志输出到文件，用于调试关闭流程
static QFile *s_logFile = nullptr;
static QtMessageHandler s_defaultHandler = nullptr;
static void fileMessageHandler(QtMsgType type, const QMessageLogContext &ctx, const QString &msg)
{
    // 过滤 Linux 平台不支持窗口透明度的警告
    if (type == QtWarningMsg && msg.contains(QStringLiteral("does not support setting window opacity")))
        return;

    if (s_logFile && s_logFile->isOpen()) {
        QTextStream out(s_logFile);
        QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        out << timestamp << " " << msg << "\n";
        out.flush();
    }

    // 同时输出到原始处理器（stderr）
    if (s_defaultHandler)
        s_defaultHandler(type, ctx, msg);
}

int main(int argc, char *argv[])
{
    // 设置日志文件
    s_logFile = new QFile("yefs_debug.log");
    s_logFile->open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);

    s_defaultHandler = qInstallMessageHandler(fileMessageHandler);

    QGuiApplication app(argc, argv);
    app.setWindowIcon(QIcon(QStringLiteral(":/YEFSApp/resources/images/YEFS.ico")));
    app.addLibraryPath(app.applicationDirPath());

    // 创建并初始化 YEFS 应用
    // 注册日志文件关闭为退出后例程，确保任何退出路径都能关闭文件
    qAddPostRoutine([]() {
        if (s_logFile) {
            s_logFile->close();
            delete s_logFile;
            s_logFile = nullptr;
        }
    });

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
