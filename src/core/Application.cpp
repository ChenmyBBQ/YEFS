#include "Application.h"
#include "MessageBus.h"
#include "PluginManager.h"
#include "MapLibreEngine.h"
#include "IMapParser.h"
#include "MapSourceManager.h"
#include "OnlineMapProvider.h"
#include "SettingsManager.h"
#include "UnitManager.h"
#include "AppIconManager.h"
#include "NerdIcon.h"
#include "parsers/GeoJSONParser.h"
#include "parsers/GPXParser.h"
#include "parsers/KMLParser.h"

#include <QQuickWindow>
#include <QFontDatabase>
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
    cleanup();
}

void Application::cleanup()
{
    if (!m_engine)
        return;

    qDebug() << "[Application] Cleaning up...";

    // Step 1: 先关闭插件（插件可能持有后台线程或定时器）
    // PluginManager 析构会依次调用每个插件的 shutdown()
    PluginManager::destroy();

    // Step 2: 销毁 QML 引擎，触发所有 QML 组件（包括 MapLibre Map）的销毁
    // MapLibre 内部的 RunLoop / RenderThread / HTTPFileSource 等在这里被释放
    delete m_engine;
    m_engine = nullptr;

    // Step 3: 按依赖逆序销毁剩余单例（delete nullptr 在 C++ 中是安全的无操作）
    UnitManager::destroy();
    MapSourceManager::destroy();
    OnlineMapProviderManager::destroy();
    MapParserFactory::destroy();
    MapLibreEngine::destroy();
    AppIconManager::destroy();
    NerdIcon::destroy();
    SettingsManager::destroy();   // 析构时自动保存未刷新的设置
    MessageBus::destroy();        // 最后销毁消息总线

    qDebug() << "[Application] Cleanup complete";
}

bool Application::initialize()
{
    qDebug() << "[Application] Initializing YEFS...";

    // 配置渲染
    qputenv("QSG_RENDER_LOOP", "basic");
    
    // 确保最后一个窗口关闭时退出应用
    m_app->setQuitOnLastWindowClosed(true);

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

    // 加载 Nerd Fonts 矢量图标库
    int fontId = QFontDatabase::addApplicationFont(":/YEFSApp/resources/font/SymbolsNerdFont-Regular.ttf");
    if (fontId == -1) {
        qWarning() << "[Application] Failed to load SymbolsNerdFont-Regular.ttf";
    }

    // 注册 QML 类型和单例
    registerQmlTypes();
    registerQmlSingletons();

    // 初始化并加载所有插件
    PluginManager::instance()->loadAllPlugins();

    // 初始化地图解析系统
    initializeMapParsers();

    // 设置连接
    setupConnections();

    // 连接 aboutToQuit 信号，在事件循环退出前清理资源
    connect(m_app, &QGuiApplication::aboutToQuit, this, &Application::cleanup);

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

    int exitCode = m_app->exec();
    qDebug() << "[Application] exec() returned with code:" << exitCode;
    return exitCode;
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
