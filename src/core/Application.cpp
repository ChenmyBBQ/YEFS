#include "Application.h"
#include "MessageBus.h"
#include "PluginManager.h"
#include "MapLibreEngine.h"
#include "MapParserFactory.h"
#include "MapSourceManager.h"
#include "OnlineMapProvider.h"
#include "parsers/GeoJSONParser.h"
#include "parsers/GPXParser.h"
#include "parsers/KMLParser.h"

#include <QQuickWindow>
#include <QMapLibre/Utils>
#include <QLoggingCategory>
#include <QDebug>

// HuskarUI
#include "husapp.h"

#ifdef BUILD_HUSKARUI_STATIC_LIBRARY
#include <QtQml/qqmlextensionplugin.h>
Q_IMPORT_QML_PLUGIN(HuskarUI_BasicPlugin)
#endif

namespace YEFS {

Application::Application(QGuiApplication* app, QObject* parent)
    : QObject(parent)
    , m_app(app)
{
}

Application::~Application()
{
}

bool Application::initialize()
{
    qDebug() << "[Application] Initializing YEFS...";

    // 配置渲染
    qputenv("QSG_RENDER_LOOP", "basic");

    QLoggingCategory::setFilterRules(
        "qt.location.*.debug=false\n"
        "qt.positioning.*.debug=false\n"
        "maplibre.*.debug=false");

    // 配置 MapLibre 渲染器
    const QMapLibre::RendererType rendererType = QMapLibre::supportedRendererType();
    auto graphicsApi = static_cast<QSGRendererInterface::GraphicsApi>(rendererType);
    QQuickWindow::setGraphicsApi(graphicsApi);

#ifndef QT_DEBUG
    QQuickWindow::setDefaultAlphaBuffer(true);
#endif

    // 设置应用信息
    m_app->setOrganizationName("YEFS");
    m_app->setOrganizationDomain("yefs.app");
    m_app->setApplicationName("YEFS");
    m_app->setApplicationDisplayName("YEFS - GIS Platform");
    m_app->setApplicationVersion("0.1.0");

    // 创建 QML 引擎
    m_engine = new QQmlApplicationEngine(this);
    m_engine->addImportPath(m_app->applicationDirPath());

    // 初始化 HuskarUI
    HusApp::initialize(m_engine);

    // 注册 QML 类型和单例
    registerQmlTypes();
    registerQmlSingletons();

    // 初始化插件管理器
    PluginManager::instance()->scanPlugins();

    // 初始化地图解析系统
    initializeMapParsers();

    // 设置连接
    setupConnections();

    qDebug() << "[Application] Initialization complete";
    return true;
}

int Application::run()
{
    // 加载主 QML
    const QUrl url(QStringLiteral("qrc:/YEFSApp/qml/Main.qml"));

    QObject::connect(m_engine, &QQmlApplicationEngine::objectCreated,
        m_app, [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    m_engine->load(url);

    // 发送应用就绪消息
    MessageBus::instance()->publish(Topics::APP_READY, true);

    return m_app->exec();
}

void Application::registerQmlTypes()
{
    // 核心类型已通过 QML_ELEMENT 自动注册
    qDebug() << "[Application] QML types registered";
}

void Application::registerQmlSingletons()
{
    // 单例通过 QML_SINGLETON 自动注册
    qDebug() << "[Application] QML singletons registered";
}

void Application::initializeMapParsers()
{
    qDebug() << "[Application] Initializing map parsers...";
    
    MapParserFactory* factory = MapParserFactory::instance();
    
    // 注册地图格式解析器
    factory->registerParser(new GeoJSONParser());
    factory->registerParser(new GPXParser());
    factory->registerParser(new KMLParser());
    
    qDebug() << "[Application] Registered parsers:" << factory->supportedExtensions();
    
    // 初始化地图源管理器
    MapSourceManager::instance();
    
    // 初始化在线地图提供商管理器
    OnlineMapProviderManager::instance();
}

void Application::setupConnections()
{
    // 这里可以设置全局信号连接
}

MessageBus* Application::messageBus()
{
    return MessageBus::instance();
}

PluginManager* Application::pluginManager()
{
    return PluginManager::instance();
}

MapLibreEngine* Application::mapEngine()
{
    return MapLibreEngine::instance();
}

} // namespace YEFS
