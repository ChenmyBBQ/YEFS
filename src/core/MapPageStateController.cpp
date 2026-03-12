#include "MapPageStateController.h"

#include "CoordinateConverter.h"
#include "MapLibreEngine.h"
#include "MessageBus.h"

#include <QGeoCoordinate>
#include <QVariantMap>

namespace YEFS {

MapPageStateController* MapPageStateController::s_instance = nullptr;

static QGeoCoordinate coordinateFromScreenPoint(const QPointF& position)
{
    return MapLibreEngine::instance()->screenToCoordinate(position.x(), position.y());
}

MapPageStateController* MapPageStateController::instance()
{
    if (!s_instance) {
        s_instance = new MapPageStateController();
    }
    return s_instance;
}

MapPageStateController* MapPageStateController::create(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    return instance();
}

void MapPageStateController::destroy()
{
    delete s_instance;
    s_instance = nullptr;
}

MapPageStateController::MapPageStateController(QObject* parent)
    : QObject(parent)
    , m_mouseLatLonText(QStringLiteral("--"))
    , m_mouseUtmText(QStringLiteral("--"))
    , m_mouseMgrsText(QStringLiteral("--"))
    , m_drawingStatusText(tr("请在地图上点击选取坐标点"))
{
    initializeShapeOptions();
    connect(MessageBus::instance(), &MessageBus::message,
            this, &MapPageStateController::onBusMessage);

    // 合并高频 hover 事件：0ms 单次定时器确保每个事件循环批次只提交一次更新
    m_hoverTimer = new QTimer(this);
    m_hoverTimer->setSingleShot(true);
    m_hoverTimer->setInterval(0);
    connect(m_hoverTimer, &QTimer::timeout, this, &MapPageStateController::processHoverUpdate);
}

QString MapPageStateController::mouseLatLonText() const
{
    return m_mouseLatLonText;
}

QString MapPageStateController::mouseUtmText() const
{
    return m_mouseUtmText;
}

QString MapPageStateController::mouseMgrsText() const
{
    return m_mouseMgrsText;
}

bool MapPageStateController::airspacePanelVisible() const
{
    return m_airspacePanelVisible;
}

bool MapPageStateController::airspaceListVisible() const
{
    return m_airspaceListVisible;
}

bool MapPageStateController::drawingActive() const
{
    return m_drawingActive;
}

bool MapPageStateController::drawingRequiresFinish() const
{
    return m_drawingRequiresFinish;
}

bool MapPageStateController::newAirspaceToolbarVisible() const
{
    return m_newAirspaceToolbarVisible;
}

int MapPageStateController::selectedShapeType() const
{
    return m_selectedShapeType;
}

QString MapPageStateController::drawingStatusText() const
{
    return m_drawingStatusText;
}

QVariantList MapPageStateController::shapeOptions() const
{
    return m_shapeOptions;
}

void MapPageStateController::handleMapTap(const QPointF& position)
{
    const QGeoCoordinate coordinate = coordinateFromScreenPoint(position);
    if (!coordinate.isValid()) {
        return;
    }

    if (m_selectedShapeType >= 0 && !m_drawingActive) {
        // 首次有效地图点击：启动绘制流程，并把本次点击记为首个控制点。
        // 工具栏、面板等覆盖层点击已在 MapInteractionLayer 中被拦截。
        MessageBus::instance()->send(QStringLiteral("airspace-manager/draw"), QVariantMap{
            {QStringLiteral("shapeType"), m_selectedShapeType}
        });
        MapLibreEngine::instance()->onMapClicked(coordinate.latitude(), coordinate.longitude());
        return;
    }

    if (m_drawingActive) {
        MapLibreEngine::instance()->onMapClicked(coordinate.latitude(), coordinate.longitude());
    }
}

void MapPageStateController::handleMapDoubleTap(const QPointF& position)
{
    const QGeoCoordinate coordinate = coordinateFromScreenPoint(position);
    if (!coordinate.isValid()) {
        return;
    }

    if (m_drawingActive) {
        QVariantMap data;
        data["latitude"] = coordinate.latitude();
        data["longitude"] = coordinate.longitude();
        MessageBus::instance()->publish(Topics::MAP_DOUBLE_CLICKED, data);
    }
}

void MapPageStateController::handleMapHover(const QPointF& position)
{
    // 只存储最新位置，由 0ms 定时器在事件批次结束后统一处理一次
    // 避免快速移动时大量重复的 JSON 序列化 + GPU 数据上传
    m_pendingHoverPos = position;
    if (!m_hoverTimer->isActive()) {
        m_hoverTimer->start();
    }
}

void MapPageStateController::processHoverUpdate()
{
    // 像素距离阈值：移动不足 2px 时跳过（消除亚像素抖动）
    const QPointF delta = m_pendingHoverPos - m_lastProcessedHoverPos;
    const bool moved = (delta.x() * delta.x() + delta.y() * delta.y()) >= 4.0;

    if (!moved && m_drawingActive) {
        // 绘图时严格跳过（坐标不变则预览也不变）
        return;
    }

    const QGeoCoordinate coordinate = coordinateFromScreenPoint(m_pendingHoverPos);
    if (!coordinate.isValid()) {
        return;
    }

    m_lastProcessedHoverPos = m_pendingHoverPos;

    if (m_drawingActive) {
        // 绘图模式：优先更新预览，跳过 UTM/MGRS 转换保证帧率
        MessageBus::instance()->send(QStringLiteral("map/hovered"), QVariantMap{
            {QStringLiteral("latitude"), coordinate.latitude()},
            {QStringLiteral("longitude"), coordinate.longitude()}
        });
        // 坐标显示仅做 LatLon（轻量），UTM/MGRS 在非绘图时更新
        const QString latLonText = CoordinateConverter::instance()->formatLatLon(
            coordinate.latitude(), coordinate.longitude(), 6);
        if (m_mouseLatLonText != latLonText) {
            m_mouseLatLonText = latLonText;
            emit mouseCoordinateChanged();
        }
    } else {
        updateMouseCoordinateTexts(coordinate.latitude(), coordinate.longitude());
    }
}

void MapPageStateController::handleMapHoverChanged(bool hovered)
{
    if (hovered) {
        return;
    }

    resetMouseCoordinate();
    if (m_drawingActive) {
        MessageBus::instance()->send(QStringLiteral("map/hovered/clear"), QVariantMap{});
    }
}

void MapPageStateController::handleAirspaceMessage(const QString& topic, const QVariant& data)
{
    const QVariantMap payload = data.toMap();

    if (topic == QStringLiteral("airspace-manager/new")) {
        setAirspacePanelVisible(true);
        setAirspaceListVisible(false);
        setNewAirspaceToolbarVisible(true);
        return;
    }

    if (topic == QStringLiteral("airspace-manager/show")) {
        setAirspacePanelVisible(true);
        setAirspaceListVisible(true);
        setNewAirspaceToolbarVisible(false);
        return;
    }

    if (topic == QStringLiteral("airspace-manager/hide")) {
        setAirspacePanelVisible(false);
        setAirspaceListVisible(false);
        setNewAirspaceToolbarVisible(false);
        clearShapeSelection();
        return;
    }

    if (topic == QStringLiteral("airspace-manager/drawing-state")) {
        const bool active = payload.value(QStringLiteral("active")).toBool();
        setDrawingActive(active);
        setDrawingRequiresFinish(payload.value(QStringLiteral("requiresFinish")).toBool());

        const QString statusText = payload.value(QStringLiteral("statusText")).toString();
        if (!statusText.isEmpty()) {
            setDrawingStatusText(statusText);
        } else if (!active) {
            setDrawingStatusText(tr("请在地图上点击选取坐标点"));
        }

        if (!active) {
            clearShapeSelection();
        }
    }
}

void MapPageStateController::toggleShapeSelection(int shapeType)
{
    if (m_selectedShapeType == shapeType) {
        setSelectedShapeType(-1);
    } else {
        setSelectedShapeType(shapeType);
    }
}

void MapPageStateController::clearShapeSelection()
{
    setSelectedShapeType(-1);
}

void MapPageStateController::hideNewAirspaceToolbar()
{
    setNewAirspaceToolbarVisible(false);
}

void MapPageStateController::hideAirspaceList()
{
    setAirspaceListVisible(false);
}

void MapPageStateController::requestHideAirspacePanel()
{
    setNewAirspaceToolbarVisible(false);
    MessageBus::instance()->send(QStringLiteral("airspace-manager/hide"), QVariantMap{});
}

void MapPageStateController::resetMouseCoordinate()
{
    if (m_mouseLatLonText == QStringLiteral("--")
        && m_mouseUtmText == QStringLiteral("--")
        && m_mouseMgrsText == QStringLiteral("--")) {
        return;
    }

    m_mouseLatLonText = QStringLiteral("--");
    m_mouseUtmText = QStringLiteral("--");
    m_mouseMgrsText = QStringLiteral("--");
    emit mouseCoordinateChanged();
}

void MapPageStateController::finishDrawing()
{
    MessageBus::instance()->send(QStringLiteral("airspace-manager/finish"), QVariantMap{});
}

void MapPageStateController::cancelDrawing()
{
    MessageBus::instance()->send(QStringLiteral("airspace-manager/cancel"), QVariantMap{});
    setDrawingActive(false);
    setDrawingRequiresFinish(false);
    setDrawingStatusText(tr("请在地图上点击选取坐标点"));
    clearShapeSelection();
}

void MapPageStateController::updateMouseCoordinateTexts(double latitude, double longitude)
{
    CoordinateConverter* converter = CoordinateConverter::instance();
    const QString latLonText = converter->formatLatLon(latitude, longitude, 6);
    const QString utmText = converter->latLonToUtmText(latitude, longitude);
    const QString mgrsText = converter->latLonToMgrsText(latitude, longitude);

    if (m_mouseLatLonText == latLonText
        && m_mouseUtmText == utmText
        && m_mouseMgrsText == mgrsText) {
        return;
    }

    m_mouseLatLonText = latLonText;
    m_mouseUtmText = utmText;
    m_mouseMgrsText = mgrsText;
    emit mouseCoordinateChanged();
}

void MapPageStateController::setAirspacePanelVisible(bool visible)
{
    if (m_airspacePanelVisible == visible) {
        return;
    }

    m_airspacePanelVisible = visible;
    emit airspacePanelVisibleChanged();
}

void MapPageStateController::setAirspaceListVisible(bool visible)
{
    if (m_airspaceListVisible == visible) {
        return;
    }

    m_airspaceListVisible = visible;
    emit airspaceListVisibleChanged();
}

void MapPageStateController::setDrawingActive(bool active)
{
    if (m_drawingActive == active) {
        return;
    }

    m_drawingActive = active;
    emit drawingActiveChanged();
}

void MapPageStateController::setDrawingRequiresFinish(bool requiresFinish)
{
    if (m_drawingRequiresFinish == requiresFinish) {
        return;
    }

    m_drawingRequiresFinish = requiresFinish;
    emit drawingRequiresFinishChanged();
}

void MapPageStateController::setNewAirspaceToolbarVisible(bool visible)
{
    if (m_newAirspaceToolbarVisible == visible) {
        return;
    }

    m_newAirspaceToolbarVisible = visible;
    emit newAirspaceToolbarVisibleChanged();
}

void MapPageStateController::setSelectedShapeType(int shapeType)
{
    if (m_selectedShapeType == shapeType) {
        return;
    }

    m_selectedShapeType = shapeType;
    emit selectedShapeTypeChanged();
}

void MapPageStateController::setDrawingStatusText(const QString& statusText)
{
    if (m_drawingStatusText == statusText) {
        return;
    }

    m_drawingStatusText = statusText;
    emit drawingStatusTextChanged();
}

void MapPageStateController::initializeShapeOptions()
{
    m_shapeOptions = {
        QVariantMap{{QStringLiteral("name"), tr("矩形")}, {QStringLiteral("type"), 0}, {QStringLiteral("iconSource"), QStringLiteral("qrc:/YEFSApp/resources/icons/shapes/shape_rectangle.svg")}},
        QVariantMap{{QStringLiteral("name"), tr("正方形")}, {QStringLiteral("type"), 1}, {QStringLiteral("iconSource"), QStringLiteral("qrc:/YEFSApp/resources/icons/shapes/shape_square.svg")}},
        QVariantMap{{QStringLiteral("name"), tr("圆形")}, {QStringLiteral("type"), 2}, {QStringLiteral("iconSource"), QStringLiteral("qrc:/YEFSApp/resources/icons/shapes/shape_circle.svg")}},
        QVariantMap{{QStringLiteral("name"), tr("多边形")}, {QStringLiteral("type"), 3}, {QStringLiteral("iconSource"), QStringLiteral("qrc:/YEFSApp/resources/icons/shapes/shape_polygon.svg")}},
        QVariantMap{{QStringLiteral("name"), tr("边界线")}, {QStringLiteral("type"), 4}, {QStringLiteral("iconSource"), QStringLiteral("qrc:/YEFSApp/resources/icons/shapes/shape_line.svg")}},
        QVariantMap{{QStringLiteral("name"), tr("圆环")}, {QStringLiteral("type"), 5}, {QStringLiteral("iconSource"), QStringLiteral("qrc:/YEFSApp/resources/icons/shapes/shape_ring.svg")}},
        QVariantMap{{QStringLiteral("name"), tr("圆弧")}, {QStringLiteral("type"), 6}, {QStringLiteral("iconSource"), QStringLiteral("qrc:/YEFSApp/resources/icons/shapes/shape_arc.svg")}},
        QVariantMap{{QStringLiteral("name"), tr("扇形")}, {QStringLiteral("type"), 7}, {QStringLiteral("iconSource"), QStringLiteral("qrc:/YEFSApp/resources/icons/shapes/shape_sector.svg")}},
        QVariantMap{{QStringLiteral("name"), tr("扇环")}, {QStringLiteral("type"), 8}, {QStringLiteral("iconSource"), QStringLiteral("qrc:/YEFSApp/resources/icons/shapes/shape_pie_segment.svg")}}
    };
}

void MapPageStateController::onBusMessage(const QString& topic, const QVariant& data)
{
    if (!topic.startsWith(QStringLiteral("airspace-manager/"))) {
        return;
    }

    handleAirspaceMessage(topic, data);
}

} // namespace YEFS