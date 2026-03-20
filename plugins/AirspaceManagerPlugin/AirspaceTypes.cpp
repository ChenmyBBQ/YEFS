#include "AirspaceTypes.h"
#include <QJsonArray>

QVariantList AirspaceGeoPoint::toVariantList() const
{
    return { latitude, longitude };
}

AirspaceGeoPoint AirspaceGeoPoint::fromVariant(const QVariant& value)
{
    const QVariantList list = value.toList();
    if (list.size() < 2) {
        return {};
    }

    AirspaceGeoPoint point;
    point.latitude = list[0].toDouble();
    point.longitude = list[1].toDouble();
    return point;
}

QVariantList AirspaceGeometrySpec::toVariantList() const
{
    QVariantList result;
    result.reserve(controlPoints.size());
    for (const AirspaceGeoPoint& point : controlPoints) {
        result.append(QVariant(point.toVariantList()));
    }
    return result;
}

QJsonObject AirspaceGeometrySpec::toJsonObject() const
{
    QJsonArray pts;
    for (const auto& p : controlPoints) {
        pts.append(QJsonArray{p.latitude, p.longitude});
    }
    return QJsonObject{
        {"shapeType", static_cast<int>(shapeType)},
        {"controlPoints", pts},
        {"previewSegments", previewSegments}
    };
}

AirspaceGeometrySpec AirspaceGeometrySpec::fromJsonObject(const QJsonObject& json)
{
    AirspaceGeometrySpec spec;
    spec.shapeType = static_cast<AirspaceShapeType>(json.value("shapeType").toInt());
    spec.previewSegments = json.value("previewSegments").toInt(64);
    QJsonArray pts = json.value("controlPoints").toArray();
    for (const auto& p : pts) {
        QJsonArray arr = p.toArray();
        if (arr.size() >= 2) {
            spec.controlPoints.append({arr[0].toDouble(), arr[1].toDouble()});
        }
    }
    return spec;
}

void AirspaceDraft::clear()
{
    geometry.controlPoints.clear();
    hasHoverPoint = false;
    hoverPoint = {};
    shapeInfo.clear();
}

QVariantList AirspaceDraft::collectedPoints() const
{
    return geometry.toVariantList();
}

QVariantList AirspaceDraft::previewPoints() const
{
    QVariantList points = collectedPoints();
    if (hasHoverPoint) {
        points.append(QVariant(hoverPoint.toVariantList()));
    }
    return points;
}

AirspaceShapeType airspaceShapeTypeFromInt(int value)
{
    switch (value) {
    case 0: return AirspaceShapeType::Rectangle;
    case 1: return AirspaceShapeType::Square;
    case 2: return AirspaceShapeType::Circle;
    case 3: return AirspaceShapeType::Polygon;
    case 4: return AirspaceShapeType::Boundary;
    case 5: return AirspaceShapeType::Ring;
    case 6: return AirspaceShapeType::Arc;
    case 7: return AirspaceShapeType::Sector;
    case 8: return AirspaceShapeType::SectorRing;
    default: return AirspaceShapeType::Rectangle;
    }
}

int airspaceShapeTypeToInt(AirspaceShapeType value)
{
    return static_cast<int>(value);
}

QString airspaceShapeTypeDisplayName(AirspaceShapeType value)
{
    switch (value) {
    case AirspaceShapeType::Rectangle: return QStringLiteral("矩形");
    case AirspaceShapeType::Square: return QStringLiteral("正方形");
    case AirspaceShapeType::Circle: return QStringLiteral("圆形");
    case AirspaceShapeType::Polygon: return QStringLiteral("多边形");
    case AirspaceShapeType::Boundary: return QStringLiteral("边界线");
    case AirspaceShapeType::Ring: return QStringLiteral("圆环");
    case AirspaceShapeType::Arc: return QStringLiteral("圆弧");
    case AirspaceShapeType::Sector: return QStringLiteral("扇形");
    case AirspaceShapeType::SectorRing: return QStringLiteral("扇环形");
    }

    return QStringLiteral("未知");
}

QVariantMap AirspaceStyle::toVariantMap() const
{
    return {
        { "fill-color", fillColor },
        { "fill-opacity", fillOpacity },
        { "line-color", lineColor },
        { "line-width", lineWidth }
    };
}

AirspaceStyle AirspaceStyle::fromVariantMap(const QVariantMap& map)
{
    AirspaceStyle style;
    if (map.contains("fill-color")) { style.fillColor = map.value("fill-color").toString(); }
    if (map.contains("fill-opacity")) { style.fillOpacity = map.value("fill-opacity").toDouble(); }
    if (map.contains("line-color")) { style.lineColor = map.value("line-color").toString(); }
    if (map.contains("line-width")) { style.lineWidth = map.value("line-width").toDouble(); }
    return style;
}

QVariantMap AirspaceProperties::toVariantMap() const
{
    return {
        { "airspaceType", airspaceType },
        { "minAltitude", minAltitude },
        { "maxAltitude", maxAltitude },
        { "effectiveTime", effectiveTime },
        { "expirationTime", expirationTime },
        { "remarks", remarks }
    };
}

AirspaceProperties AirspaceProperties::fromVariantMap(const QVariantMap& map)
{
    AirspaceProperties props;
    if (map.contains("airspaceType")) { props.airspaceType = map.value("airspaceType").toInt(); }
    if (map.contains("minAltitude")) { props.minAltitude = map.value("minAltitude").toDouble(); }
    if (map.contains("maxAltitude")) { props.maxAltitude = map.value("maxAltitude").toDouble(); }
    if (map.contains("effectiveTime")) { props.effectiveTime = map.value("effectiveTime").toString(); }
    if (map.contains("expirationTime")) { props.expirationTime = map.value("expirationTime").toString(); }
    if (map.contains("remarks")) { props.remarks = map.value("remarks").toString(); }
    return props;
}