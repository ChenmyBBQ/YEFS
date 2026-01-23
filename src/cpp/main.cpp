
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

int main(int argc, char *argv[])
{
    // Fix for MapLibre assertion failure on Linux Debug builds
    qputenv("QSG_RENDER_LOOP", "basic");

    QLoggingCategory::setFilterRules(
        "qt.location.*.debug=true\n"
        "qt.positioning.*.debug=true\n"
        "maplibre.*.debug=true");

    const QMapLibre::RendererType rendererType = QMapLibre::supportedRendererType();
    auto graphicsApi = static_cast<QSGRendererInterface::GraphicsApi>(rendererType);
    QQuickWindow::setGraphicsApi(graphicsApi);

#ifndef QT_DEBUG
    // Enable Alpha Buffer in Release to support rounded corners/transparency
    QQuickWindow::setDefaultAlphaBuffer(true);
#endif

    QGuiApplication app(argc, argv);
    app.addLibraryPath(app.applicationDirPath());
    
    app.setOrganizationName("MenPenS");
    app.setApplicationName("HuskarUI");
    app.setApplicationDisplayName("HuskarUI Gallery");
    app.setApplicationVersion(HusApp::libVersion());

    QQmlApplicationEngine engine;
    // Ensure the application can find the QML modules in the bin directory
    engine.addImportPath(app.applicationDirPath());

    HusApp::initialize(&engine);
    CustomTheme::instance()->registerAll();

    const QUrl url = QUrl(QStringLiteral("qrc:/Gallery/qml/Gallery.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);
    // engine.addImportPath(HUSKARUI_IMPORT_PATH);
    engine.load(url);

    return app.exec();
}
