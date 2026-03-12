#include "DrawingController.h"
#include "ShapeGenerator.h"
#include "AirspaceModel.h"

#include <QtMath>
#include <QJsonDocument>
#include <QVariantMap>
#include <QDebug>

// ============================================================================
// Haversine helpers
// ============================================================================

double DrawingController::haversineDistance(double lat1, double lng1,
                                            double lat2, double lng2)
{
    constexpr double R = 6371000.0;
    double dLat = qDegreesToRadians(lat2 - lat1);
    double dLng = qDegreesToRadians(lng2 - lng1);
    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
               std::cos(qDegreesToRadians(lat1)) * std::cos(qDegreesToRadians(lat2)) *
               std::sin(dLng / 2) * std::sin(dLng / 2);
    return R * 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
}

double DrawingController::bearingTo(double lat1, double lng1,
                                     double lat2, double lng2)
{
    double dLng = qDegreesToRadians(lng2 - lng1);
    double y = std::sin(dLng) * std::cos(qDegreesToRadians(lat2));
    double x = std::cos(qDegreesToRadians(lat1)) * std::sin(qDegreesToRadians(lat2)) -
               std::sin(qDegreesToRadians(lat1)) * std::cos(qDegreesToRadians(lat2)) * std::cos(dLng);
    return std::fmod(qRadiansToDegrees(std::atan2(y, x)) + 360.0, 360.0);
}

// ============================================================================
// 构造
// ============================================================================

DrawingController::DrawingController(ShapeGenerator* shapeGen,
                                      AirspaceModel* model,
                                      QObject* parent)
    : QObject(parent)
    , m_shapeGen(shapeGen)
    , m_model(model)
{
}

int DrawingController::requiredPoints() const { return minPointsForShape(); }

int DrawingController::minPointsForShape() const
{
    switch (m_shapeType) {
    case 0: return 2;  // Rectangle
    case 1: return 2;  // Square
    case 2: return 2;  // Circle
    case 3: return -1; // Polygon (>=3, 手动结束)
    case 4: return -1; // Boundary (>=2, 手动结束)
    case 5: return 3;  // Ring
    case 6: return 4;  // Arc
    case 7: return 4;  // Sector
    case 8: return 5;  // SectorRing
    default: return -1;
    }
}

// ============================================================================
// 绘制生命周期
// ============================================================================

void DrawingController::startDrawing(int shapeType)
{
    cancel();
    m_shapeType = shapeType;
    m_state = Drawing;
    m_points.clear();
    m_hoverPoint.clear();
    m_hasHoverPoint = false;
    m_previewGeoJson = {};

    emit currentShapeTypeChanged(m_shapeType);
    emit drawingStateChanged(m_state);
    emit pointsChanged();
    setStatusText(QStringLiteral("左键点击取点，按住拖动平移地图，右键取消"));
}

void DrawingController::addPoint(double latitude, double longitude)
{
    if (m_state != Drawing) return;

    int minPts = minPointsForShape();
    int n = m_points.size();

    // 如果是定点数图形且点数已达到，用新点击点替换最后一个点（以便连续调整）
    if (minPts > 0 && n >= minPts) {
        m_points[n - 1] = QVariant(QVariantList{ latitude, longitude });
        emit pointsChanged();
        updatePreview();
        return;
    }

    m_points.append(QVariant(QVariantList{ latitude, longitude }));
    clearHoverPoint();
    emit pointsChanged();

    n = m_points.size();

    if (minPts > 0) {
        int rem = minPts - n;
        if (rem > 0)
            setStatusText(QStringLiteral("还需要 %1 个点（右键可取消）").arg(rem));
        else
            setStatusText(QStringLiteral("绘制中，双击或在右侧面板点击确认结束"));
    } else {
        setStatusText(QStringLiteral("已采集 %1 个点，双击或在右侧面板点击确认结束（右键可取消）").arg(n));
    }

    updatePreview();
}

void DrawingController::undoLastPoint()
{
    if (m_state != Drawing || m_points.isEmpty()) return;
    m_points.removeLast();
    emit pointsChanged();
    updatePreview();
    setStatusText(QStringLiteral("已撤销，当前 %1 个点").arg(m_points.size()));
}

void DrawingController::finishDrawing()
{
    if (m_state != Drawing) return;

    int n = m_points.size();
    if (m_shapeType == 3 && n < 3) {
        setStatusText(QStringLiteral("多边形至少需要 3 个点")); return;
    }
    if (m_shapeType == 4 && n < 2) {
        setStatusText(QStringLiteral("边界线至少需要 2 个点")); return;
    }

    clearHoverPoint();
    updatePreview();
    m_state = Editing;
    emit drawingStateChanged(m_state);
    setStatusText(QStringLiteral("绘制完成，请编辑属性"));
    emit drawingCompleted(m_previewGeoJson, m_shapeType);
}

void DrawingController::cancel()
{
    if (m_state == Idle) return;
    m_state = Idle;
    m_shapeType = -1;
    m_points.clear();
    m_hoverPoint.clear();
    m_hasHoverPoint = false;
    m_previewGeoJson = {};

    emit drawingStateChanged(m_state);
    emit currentShapeTypeChanged(m_shapeType);
    emit pointsChanged();
    emit drawingCancelled();
    setStatusText({});
}

QString DrawingController::saveAirspace(const QString& name,
                                         const QString& styleJson,
                                         const QString& propertiesJson)
{
    if (m_previewGeoJson.isEmpty()) return {};

    QString geoJsonStr = QString::fromUtf8(
        QJsonDocument(m_previewGeoJson).toJson(QJsonDocument::Compact));

    QString uuid = m_model->addAirspace(name, m_shapeType,
                                         geoJsonStr, styleJson, propertiesJson);
    if (!uuid.isEmpty()) {
        m_state = Idle;
        m_shapeType = -1;
        m_points.clear();
        m_previewGeoJson = {};
        emit drawingStateChanged(m_state);
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
    }
}

void DrawingController::updateHoverPoint(double latitude, double longitude)
{
    if (m_state != Drawing)
        return;

    m_hoverPoint = QVariantList{ latitude, longitude };
    m_hasHoverPoint = true;
    updatePreview();
}

void DrawingController::clearHoverPoint()
{
    m_hoverPoint.clear();
    m_hasHoverPoint = false;
}

// ============================================================================
// 预览
// ============================================================================

static QVariantList extractCoordsFromGeoJson(const QJsonObject& geoJson) {
    QVariantList res;
    if (!geoJson.contains("geometry")) return res;
    QJsonObject geom = geoJson["geometry"].toObject();
    QString type = geom["type"].toString();
    QJsonArray coords = geom["coordinates"].toArray();

    auto addPoints = [&](const QJsonArray& arr) {
        for (int i = 0; i < arr.size(); ++i) {
            QJsonArray pt = arr[i].toArray(); // geojson: [lng, lat]
            if (pt.size() >= 2) {
                // setPreviewAnnotation 需要: [lat, lng]
                res.append(QVariant(QVariantList{pt[1].toDouble(), pt[0].toDouble()}));
            }
        }
    };

    if (type == QLatin1String("Polygon")) {
        if (!coords.isEmpty()) addPoints(coords[0].toArray()); // 外环
    } else if (type == QLatin1String("LineString")) {
        addPoints(coords);
    }
    return res;
}

void DrawingController::updatePreview()
{
    QVariantList previewPoints = m_points;
    if (m_state == Drawing && m_hasHoverPoint && m_hoverPoint.size() == 2)
        previewPoints.append(QVariant(m_hoverPoint));

    int n = previewPoints.size();
    if (n == 0) {
        m_previewGeoJson = {};
        emit previewAnnotationCleared();
        return;
    }
    auto lat = [&previewPoints](int i) { return previewPoints[i].toList()[0].toDouble(); };
    auto lng = [&previewPoints](int i) { return previewPoints[i].toList()[1].toDouble(); };

    const int segs = m_hasHoverPoint ? 32 : 64;
    QVariantMap shapeInfo;
    shapeInfo["type"] = m_shapeType;
    shapeInfo["pointCount"] = n;

    switch (m_shapeType) {
    case 0: // Rectangle
    case 1: // Square
        if (n >= 2) {
            m_previewGeoJson = m_shapeGen->rectangleFromCorners(lat(0), lng(0), lat(1), lng(1));
            shapeInfo["centerLat"] = (lat(0) + lat(1)) / 2.0;
            shapeInfo["centerLng"] = (lng(0) + lng(1)) / 2.0;
            shapeInfo["diagonalMeters"] = haversineDistance(lat(0), lng(0), lat(1), lng(1));
        }
        break;
    case 2: // Circle
        if (n >= 2) {
            m_previewGeoJson = m_shapeGen->circleFromCenterEdge(lat(0), lng(0), lat(1), lng(1), segs);
            shapeInfo["centerLat"] = lat(0);
            shapeInfo["centerLng"] = lng(0);
            shapeInfo["radiusMeters"] = haversineDistance(lat(0), lng(0), lat(1), lng(1));
        } else {
            m_previewGeoJson = m_shapeGen->generateCircle(lat(0), lng(0), 100, segs);
            shapeInfo["centerLat"] = lat(0);
            shapeInfo["centerLng"] = lng(0);
            shapeInfo["radiusMeters"] = 100;
        }
        break;
    case 3: // Polygon
        if (n >= 3) m_previewGeoJson = m_shapeGen->generatePolygon(previewPoints);
        else if (n >= 2) m_previewGeoJson = m_shapeGen->generateBoundary(previewPoints);
        break;
    case 4: // Boundary
        if (n >= 2) m_previewGeoJson = m_shapeGen->generateBoundary(previewPoints);
        break;
    case 5: { // Ring
        if (n >= 3) {
            double oR = haversineDistance(lat(0), lng(0), lat(1), lng(1));
            double iR = haversineDistance(lat(0), lng(0), lat(2), lng(2));
            m_previewGeoJson = m_shapeGen->generateRing(lat(0), lng(0), oR, iR, segs);
            shapeInfo["outerRadius"] = oR;
            shapeInfo["innerRadius"] = iR;
        } else if (n == 2) {
            m_previewGeoJson = m_shapeGen->circleFromCenterEdge(lat(0), lng(0), lat(1), lng(1), segs);
            shapeInfo["outerRadius"] = haversineDistance(lat(0), lng(0), lat(1), lng(1));
        }
        break;
    }
    case 6: { // Arc
        double r = (n >= 2) ? haversineDistance(lat(0), lng(0), lat(1), lng(1)) : 0;
        if (n >= 4) {
            double sa = bearingTo(lat(0), lng(0), lat(2), lng(2));
            double ea = bearingTo(lat(0), lng(0), lat(3), lng(3));
            m_previewGeoJson = m_shapeGen->generateArc(lat(0), lng(0), r, sa, ea, segs);
            shapeInfo["startAngle"] = sa;
            shapeInfo["endAngle"] = ea;
        } else if (n >= 2) {
            m_previewGeoJson = m_shapeGen->circleFromCenterEdge(lat(0), lng(0), lat(1), lng(1), segs);
        }
        shapeInfo["radius"] = r;
        break;
    }
    case 7: { // Sector
        double r = (n >= 2) ? haversineDistance(lat(0), lng(0), lat(1), lng(1)) : 0;
        if (n >= 4) {
            double sa = bearingTo(lat(0), lng(0), lat(2), lng(2));
            double ea = bearingTo(lat(0), lng(0), lat(3), lng(3));
            m_previewGeoJson = m_shapeGen->generateSector(lat(0), lng(0), r, sa, ea, segs);
            shapeInfo["startAngle"] = sa;
            shapeInfo["endAngle"] = ea;
        } else if (n >= 2) {
            m_previewGeoJson = m_shapeGen->circleFromCenterEdge(lat(0), lng(0), lat(1), lng(1), segs);
        }
        shapeInfo["radius"] = r;
        break;
    }
    case 8: { // SectorRing
        if (n >= 5) {
            double oR = haversineDistance(lat(0), lng(0), lat(1), lng(1));
            double iR = haversineDistance(lat(0), lng(0), lat(2), lng(2));
            double sa = bearingTo(lat(0), lng(0), lat(3), lng(3));
            double ea = bearingTo(lat(0), lng(0), lat(4), lng(4));
            m_previewGeoJson = m_shapeGen->generateSectorRing(lat(0), lng(0), oR, iR, sa, ea, segs);
            shapeInfo["outerRadius"] = oR;
            shapeInfo["innerRadius"] = iR;
            shapeInfo["startAngle"] = sa;
            shapeInfo["endAngle"] = ea;
        } else if (n >= 3) {
            double oR = haversineDistance(lat(0), lng(0), lat(1), lng(1));
            double iR = haversineDistance(lat(0), lng(0), lat(2), lng(2));
            m_previewGeoJson = m_shapeGen->generateRing(lat(0), lng(0), oR, iR, segs);
            shapeInfo["outerRadius"] = oR;
            shapeInfo["innerRadius"] = iR;
        } else if (n >= 2) {
            m_previewGeoJson = m_shapeGen->circleFromCenterEdge(lat(0), lng(0), lat(1), lng(1), segs);
            shapeInfo["outerRadius"] = haversineDistance(lat(0), lng(0), lat(1), lng(1));
        }
        break;
    }
    default: break;
    }

    if (m_currentShapeInfo != shapeInfo) {
        m_currentShapeInfo = shapeInfo;
        emit currentShapeInfoChanged(shapeInfo);
    }

    // hover 状态只走内存高频渲染，不抛出 QML 事件更新普通 JSON 图层
    if (m_state == Drawing && m_hasHoverPoint) {
        QVariantList annPoints = extractCoordsFromGeoJson(m_previewGeoJson);
        emit previewAnnotationSet(annPoints);
    } else {
        // 完成绘制或右键取消等，抛出让 QML 的常规 GeoJSON 图层承接，并清除快速渲染层
        emit previewAnnotationCleared();
        emit previewUpdated(m_previewGeoJson);
    }
}

void DrawingController::setStatusText(const QString& text)
{
    if (m_statusText != text) {
        m_statusText = text;
        emit statusTextChanged();
    }
}
