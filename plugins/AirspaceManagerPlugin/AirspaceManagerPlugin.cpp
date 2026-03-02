#include "AirspaceManagerPlugin.h"
#include "AirspaceDatabase.h"
#include "AirspaceModel.h"
#include "ShapeGenerator.h"
#include "DrawingController.h"

#include <QDebug>
#include <QJsonDocument>

AirspaceManagerPlugin::AirspaceManagerPlugin(QObject* parent)
    : IPlugin(parent)
{
}

AirspaceManagerPlugin::~AirspaceManagerPlugin()
{
    shutdown();
}

bool AirspaceManagerPlugin::initialize(YEFS::PluginContext* context)
{
    m_context = context;

    qDebug() << "[AirspaceManagerPlugin] Initializing...";

    // 创建数据库
    m_database = new AirspaceDatabase(this);
    if (!m_database->open()) {
        qWarning() << "[AirspaceManagerPlugin] Failed to open database";
        return false;
    }

    // 创建数据模型
    m_model = new AirspaceModel(m_database, this);
    m_model->loadFromDatabase();

    // 创建形状生成器
    m_shapeGen = new ShapeGenerator(this);

    // 创建绘制控制器
    m_drawCtrl = new DrawingController(m_shapeGen, m_model, this);

    // 如果有消息总线，连接地图点击事件和插件命令
    if (m_context) {
        auto* msgBus = m_context->getService("MessageBus");
        if (msgBus) {
            connect(msgBus, SIGNAL(message(QString,QVariant)),
                    m_drawCtrl, SLOT(onMessage(QString,QVariant)));

            // 绘制状态变化时通知 MapPage
            connect(m_drawCtrl, &DrawingController::drawingStateChanged,
                    this, [msgBus, this](int state) {
                QVariantMap stateData;
                stateData["active"] = (state == DrawingController::Drawing);
                stateData["statusText"] = m_drawCtrl->statusText();
                stateData["requiresFinish"] = (m_drawCtrl->requiredPoints() < 0);
                QMetaObject::invokeMethod(msgBus, "publish",
                    Q_ARG(QString, QStringLiteral("airspace-manager/drawing-state")),
                    Q_ARG(QVariant, QVariant::fromValue(stateData)));
            });

            // 状态文本变化时也通知
            connect(m_drawCtrl, &DrawingController::statusTextChanged,
                    this, [msgBus, this]() {
                if (m_drawCtrl->drawingState() != DrawingController::Idle) {
                    QVariantMap stateData;
                    stateData["active"] = (m_drawCtrl->drawingState() == DrawingController::Drawing);
                    stateData["statusText"] = m_drawCtrl->statusText();
                    stateData["requiresFinish"] = (m_drawCtrl->requiredPoints() < 0);
                    QMetaObject::invokeMethod(msgBus, "publish",
                        Q_ARG(QString, QStringLiteral("airspace-manager/drawing-state")),
                        Q_ARG(QVariant, QVariant::fromValue(stateData)));
                }
            });
        }
    }

    // 注册 QML 类型供插件页面使用
    registerQmlTypes();

    // 将已保存的空域加载到地图
    loadAirspacesToMap();

    qDebug() << "[AirspaceManagerPlugin] Initialized successfully";
    return true;
}

void AirspaceManagerPlugin::shutdown()
{
    qDebug() << "[AirspaceManagerPlugin] Shutting down...";

    if (m_drawCtrl) {
        m_drawCtrl->cancel();
    }
    if (m_database) {
        m_database->close();
    }
}

QUrl AirspaceManagerPlugin::qmlEntry() const
{
    return QUrl(QStringLiteral("qrc:/AirspaceManagerPlugin/qml/AirspaceManagerPage.qml"));
}

QUrl AirspaceManagerPlugin::settingsPage() const
{
    return QUrl(QStringLiteral("qrc:/AirspaceManagerPlugin/qml/AirspaceSettings.qml"));
}

QUrl AirspaceManagerPlugin::toolbarItem() const
{
    return QUrl(QStringLiteral("qrc:/AirspaceManagerPlugin/qml/AirspaceToolbar.qml"));
}

QUrl AirspaceManagerPlugin::sidePanel() const
{
    return QUrl(QStringLiteral("qrc:/AirspaceManagerPlugin/qml/AirspaceListPanel.qml"));
}

void AirspaceManagerPlugin::registerQmlTypes()
{
    // 将实例注册为 QML context property
    // 这些实例通过插件管理器的 getPlugin() 获取后在 QML 中使用
    qmlRegisterSingletonInstance("AirspaceManager", 1, 0, "AirspaceDB", m_database);
    qmlRegisterSingletonInstance("AirspaceManager", 1, 0, "AirspaceModel", m_model);
    qmlRegisterSingletonInstance("AirspaceManager", 1, 0, "ShapeGen", m_shapeGen);
    qmlRegisterSingletonInstance("AirspaceManager", 1, 0, "DrawCtrl", m_drawCtrl);
}

void AirspaceManagerPlugin::loadAirspacesToMap()
{
    // 将已保存的空域加载到地图图层（通过 MapLibreEngine）
    if (!m_context) return;

    auto* engine = m_context->getService("MapLibreEngine");
    if (!engine) return;

    auto airspaces = m_database->getAllAirspaces();
    for (const auto& a : airspaces) {
        QJsonObject geoJson = QJsonDocument::fromJson(a.geoJson.toUtf8()).object();
        QVariantMap style = QJsonDocument::fromJson(a.styleJson.toUtf8()).object().toVariantMap();
        QMetaObject::invokeMethod(engine, "addGeoJSONLayer",
                                  Q_ARG(QString, "airspace-" + a.id),
                                  Q_ARG(QJsonObject, geoJson),
                                  Q_ARG(QVariantMap, style));
    }
}
