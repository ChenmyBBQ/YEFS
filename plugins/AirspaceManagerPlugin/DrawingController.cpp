#include "DrawingController.h"
#include "AirspaceDraftValidator.h"
#include "ShapeGenerator.h"
#include "AirspaceModel.h"

#include <QJsonDocument>
#include <QVariantMap>

// ============================================================================
// 构造
// ============================================================================

DrawingController::DrawingController(ShapeGenerator* shapeGen,
                                      AirspaceModel* model,
                                      QObject* parent)
    : QObject(parent)
    , m_shapeGen(shapeGen)
    , m_model(model)
    , m_geometryConverter(shapeGen)
{
}

int DrawingController::requiredPoints() const { return AirspaceDraftValidator::minimumPoints(m_draft.geometry.shapeType); }

// ============================================================================
// 绘制生命周期
// ============================================================================

void DrawingController::startDrawing(int shapeType)
{
    cancel();
    m_draft.clear();
    m_draft.geometry.shapeType = airspaceShapeTypeFromInt(shapeType);
    m_state = Drawing;

    emit currentShapeTypeChanged(currentShapeType());
    emit drawingStateChanged(m_state);
    emit pointsChanged();
    setStatusText(QStringLiteral("左键点击取点，按住拖动平移地图，右键取消"));
}

void DrawingController::addPoint(double latitude, double longitude)
{
    if (m_state != Drawing) return;

    const int minPts = requiredPoints();
    const int currentCount = m_draft.geometry.controlPoints.size();
    const AirspaceGeoPoint point{ latitude, longitude };

    // 如果是定点数图形且点数已达到，用新点击点替换最后一个点（以便连续调整）
    if (minPts > 0 && currentCount >= minPts) {
        m_draft.geometry.controlPoints[currentCount - 1] = point;
        emit pointsChanged();
        updatePreview();
        return;
    }

    m_draft.geometry.controlPoints.append(point);
    clearHoverPoint();
    emit pointsChanged();

    const int pointCount = m_draft.geometry.controlPoints.size();

    if (minPts > 0) {
        const int rem = minPts - pointCount;
        if (rem > 0)
            setStatusText(QStringLiteral("还需要 %1 个点（右键可取消）").arg(rem));
        else
            setStatusText(QStringLiteral("绘制中，双击或在右侧面板点击确认结束"));
    } else {
        setStatusText(QStringLiteral("已采集 %1 个点，双击或在右侧面板点击确认结束（右键可取消）").arg(pointCount));
    }

    updatePreview();
}

void DrawingController::undoLastPoint()
{
    if (m_state != Drawing || m_draft.geometry.controlPoints.isEmpty()) return;
    m_draft.geometry.controlPoints.removeLast();
    emit pointsChanged();
    updatePreview();
    setStatusText(QStringLiteral("已撤销，当前 %1 个点").arg(m_draft.geometry.controlPoints.size()));
}

void DrawingController::finishDrawing()
{
    if (m_state != Drawing) return;

    const QString error = AirspaceDraftValidator::finishError(m_draft);
    if (!error.isEmpty()) {
        setStatusText(error);
        return;
    }

    clearHoverPoint();
    updatePreview();
    m_state = Editing;
    emit drawingStateChanged(m_state);
    setStatusText(QStringLiteral("绘制完成，请编辑属性"));
    emit drawingCompleted(m_geometryConverter.buildPreview(m_draft).geoJson, currentShapeType());
}

void DrawingController::cancel()
{
    if (m_state == Idle) return;
    m_state = Idle;
    m_draft.clear();
    m_draft.geometry.shapeType = AirspaceShapeType::Rectangle;

    emit drawingStateChanged(m_state);
    emit currentShapeTypeChanged(-1);
    emit pointsChanged();
    emit drawingCancelled();
    setStatusText({});
}

QString DrawingController::saveAirspace(const QString& name,
                                         const QString& styleJson,
                                         const QString& propertiesJson)
{
    return saveAirspaceData(
        name,
        QJsonDocument::fromJson(styleJson.toUtf8()).object().toVariantMap(),
        QJsonDocument::fromJson(propertiesJson.toUtf8()).object().toVariantMap());
}

QString DrawingController::saveAirspaceData(const QString& name,
                                             const QVariantMap& styleData,
                                             const QVariantMap& propertiesData)
{
    if (m_draft.geometry.controlPoints.isEmpty()) return {};

    AirspaceEntity entity;
    entity.name = name;
    entity.geometry = m_draft.geometry;
    entity.style = AirspaceStyle::fromVariantMap(styleData);
    entity.properties = AirspaceProperties::fromVariantMap(propertiesData);

    const QString uuid = m_model->addAirspace(entity);
    if (!uuid.isEmpty()) {
        m_state = Idle;
        m_draft.clear();
        m_draft.geometry.shapeType = AirspaceShapeType::Rectangle;
        emit drawingStateChanged(m_state);
        emit currentShapeTypeChanged(-1);
        emit pointsChanged();
        setStatusText(QStringLiteral("空域已保存"));
    }
    return uuid;
}

// ============================================================================
// MessageBus
// ============================================================================

void DrawingController::onMessage(const QString& topic, const QVariant& data)
{
    if (topic == QLatin1String("map/clicked") && m_state == Drawing) {
        auto map = data.toMap();
        addPoint(map.value("latitude").toDouble(),
                 map.value("longitude").toDouble());
    } else if (topic == QLatin1String("map/doubleClicked") && m_state == Drawing) {
        auto map = data.toMap();
        // Since double-click probably also fires single-click first, addPoint might have been called.
        // We just ensure we finish drawing.
        finishDrawing();
    } else if (topic == QLatin1String("map/hovered") && m_state == Drawing) {
        auto map = data.toMap();
        updateHoverPoint(map.value("latitude").toDouble(),
                         map.value("longitude").toDouble());
    } else if (topic == QLatin1String("map/hovered/clear") && m_state == Drawing) {
        clearHoverPoint();
        updatePreview();
    } else if (topic == QLatin1String("airspace-manager/draw")) {
        auto map = data.toMap();
        int shapeType = map.value("shapeType", 0).toInt();
        startDrawing(shapeType);
    } else if (topic == QLatin1String("airspace-manager/finish")) {
        finishDrawing();
    } else if (topic == QLatin1String("airspace-manager/cancel")) {
        cancel();
    } else if (topic == QLatin1String("airspace-manager/undo")) {
        undoLastPoint();
    }
}

void DrawingController::updateHoverPoint(double latitude, double longitude)
{
    if (m_state != Drawing)
        return;

    m_draft.hoverPoint = { latitude, longitude };
    m_draft.hasHoverPoint = true;
    updatePreview();
}

void DrawingController::clearHoverPoint()
{
    m_draft.hasHoverPoint = false;
    m_draft.hoverPoint = {};
}

void DrawingController::updatePreview()
{
    if (!AirspaceDraftValidator::canPreview(m_draft)) {
        m_draft.shapeInfo.clear();
        emit previewAnnotationCleared();
        return;
    }
    const AirspacePreviewResult preview = m_geometryConverter.buildPreview(m_draft);

    if (m_draft.shapeInfo != preview.shapeInfo) {
        m_draft.shapeInfo = preview.shapeInfo;
        emit currentShapeInfoChanged(preview.shapeInfo);
    }

    // hover 状态只走内存高频渲染，不抛出 QML 事件更新普通 JSON 图层
    if (m_state == Drawing && m_draft.hasHoverPoint) {
        emit previewAnnotationSet(preview.annotationPoints);
    } else {
        // 完成绘制或右键取消等，抛出让 QML 的常规 GeoJSON 图层承接，并清除快速渲染层
        emit previewAnnotationCleared();
        emit previewUpdated(preview.geoJson);
    }
}

void DrawingController::setStatusText(const QString& text)
{
    if (m_statusText != text) {
        m_statusText = text;
        emit statusTextChanged();
    }
}
