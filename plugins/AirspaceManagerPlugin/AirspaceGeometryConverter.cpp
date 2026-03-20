#include "AirspaceGeometryConverter.h"

#include "ShapeGenerator.h"

#include <QJsonArray>
#include <QtMath>

namespace {

double pointLat(const QVariantList& points, int index)
{
    return points[index].toList()[0].toDouble();
}

double pointLng(const QVariantList& points, int index)
{
    return points[index].toList()[1].toDouble();
}

} // namespace

AirspaceGeometryConverter::AirspaceGeometryConverter(const ShapeGenerator* shapeGenerator)
    : m_shapeGenerator(shapeGenerator)
{
}

void AirspaceGeometryConverter::setShapeGenerator(const ShapeGenerator* shapeGenerator)
{
    m_shapeGenerator = shapeGenerator;
}

AirspacePreviewResult AirspaceGeometryConverter::buildPreview(const AirspaceDraft& draft) const
{
    AirspacePreviewResult result;
    if (!m_shapeGenerator) {
        return result;
    }

    const QVariantList previewPoints = draft.previewPoints();
    const int count = previewPoints.size();
    if (count <= 0) {
        return result;
    }

    QVariantMap shapeInfo;
    shapeInfo[QStringLiteral("type")] = airspaceShapeTypeToInt(draft.geometry.shapeType);
    shapeInfo[QStringLiteral("pointCount")] = count;

    const int segments = draft.hasHoverPoint ? 32 : draft.geometry.previewSegments;

    switch (draft.geometry.shapeType) {
    case AirspaceShapeType::Rectangle:
    case AirspaceShapeType::Square:
        if (count >= 2) {
            result.geoJson = m_shapeGenerator->rectangleFromCorners(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0),
                pointLat(previewPoints, 1), pointLng(previewPoints, 1));
            shapeInfo[QStringLiteral("centerLat")] = (pointLat(previewPoints, 0) + pointLat(previewPoints, 1)) / 2.0;
            shapeInfo[QStringLiteral("centerLng")] = (pointLng(previewPoints, 0) + pointLng(previewPoints, 1)) / 2.0;
            shapeInfo[QStringLiteral("diagonalMeters")] = haversineDistance(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0),
                pointLat(previewPoints, 1), pointLng(previewPoints, 1));
        }
        break;
    case AirspaceShapeType::Circle:
        if (count >= 2) {
            result.geoJson = m_shapeGenerator->circleFromCenterEdge(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0),
                pointLat(previewPoints, 1), pointLng(previewPoints, 1), segments);
            shapeInfo[QStringLiteral("centerLat")] = pointLat(previewPoints, 0);
            shapeInfo[QStringLiteral("centerLng")] = pointLng(previewPoints, 0);
            shapeInfo[QStringLiteral("radiusMeters")] = haversineDistance(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0),
                pointLat(previewPoints, 1), pointLng(previewPoints, 1));
        } else {
            result.geoJson = m_shapeGenerator->generateCircle(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0), 100.0, segments);
            shapeInfo[QStringLiteral("centerLat")] = pointLat(previewPoints, 0);
            shapeInfo[QStringLiteral("centerLng")] = pointLng(previewPoints, 0);
            shapeInfo[QStringLiteral("radiusMeters")] = 100.0;
        }
        break;
    case AirspaceShapeType::Polygon:
        if (count >= 3) {
            result.geoJson = m_shapeGenerator->generatePolygon(previewPoints);
        } else if (count >= 2) {
            result.geoJson = m_shapeGenerator->generateBoundary(previewPoints);
        }
        break;
    case AirspaceShapeType::Boundary:
        if (count >= 2) {
            result.geoJson = m_shapeGenerator->generateBoundary(previewPoints);
        }
        break;
    case AirspaceShapeType::Ring:
        if (count >= 3) {
            const double outerRadius = haversineDistance(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0),
                pointLat(previewPoints, 1), pointLng(previewPoints, 1));
            const double innerRadius = haversineDistance(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0),
                pointLat(previewPoints, 2), pointLng(previewPoints, 2));
            result.geoJson = m_shapeGenerator->generateRing(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0), outerRadius, innerRadius, segments);
            shapeInfo[QStringLiteral("outerRadius")] = outerRadius;
            shapeInfo[QStringLiteral("innerRadius")] = innerRadius;
        } else if (count >= 2) {
            result.geoJson = m_shapeGenerator->circleFromCenterEdge(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0),
                pointLat(previewPoints, 1), pointLng(previewPoints, 1), segments);
            shapeInfo[QStringLiteral("outerRadius")] = haversineDistance(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0),
                pointLat(previewPoints, 1), pointLng(previewPoints, 1));
        }
        break;
    case AirspaceShapeType::Arc:
    case AirspaceShapeType::Sector: {
        const double radius = count >= 2
            ? haversineDistance(pointLat(previewPoints, 0), pointLng(previewPoints, 0),
                                pointLat(previewPoints, 1), pointLng(previewPoints, 1))
            : 0.0;
        if (count >= 4) {
            const double startAngle = bearingTo(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0),
                pointLat(previewPoints, 2), pointLng(previewPoints, 2));
            const double endAngle = bearingTo(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0),
                pointLat(previewPoints, 3), pointLng(previewPoints, 3));
            if (draft.geometry.shapeType == AirspaceShapeType::Arc) {
                result.geoJson = m_shapeGenerator->generateArc(
                    pointLat(previewPoints, 0), pointLng(previewPoints, 0), radius, startAngle, endAngle, segments);
            } else {
                result.geoJson = m_shapeGenerator->generateSector(
                    pointLat(previewPoints, 0), pointLng(previewPoints, 0), radius, startAngle, endAngle, segments);
            }
            shapeInfo[QStringLiteral("startAngle")] = startAngle;
            shapeInfo[QStringLiteral("endAngle")] = endAngle;
        } else if (count >= 2) {
            result.geoJson = m_shapeGenerator->circleFromCenterEdge(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0),
                pointLat(previewPoints, 1), pointLng(previewPoints, 1), segments);
        }
        shapeInfo[QStringLiteral("radius")] = radius;
        break;
    }
    case AirspaceShapeType::SectorRing:
        if (count >= 5) {
            const double outerRadius = haversineDistance(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0),
                pointLat(previewPoints, 1), pointLng(previewPoints, 1));
            const double innerRadius = haversineDistance(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0),
                pointLat(previewPoints, 2), pointLng(previewPoints, 2));
            const double startAngle = bearingTo(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0),
                pointLat(previewPoints, 3), pointLng(previewPoints, 3));
            const double endAngle = bearingTo(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0),
                pointLat(previewPoints, 4), pointLng(previewPoints, 4));
            result.geoJson = m_shapeGenerator->generateSectorRing(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0),
                outerRadius, innerRadius, startAngle, endAngle, segments);
            shapeInfo[QStringLiteral("outerRadius")] = outerRadius;
            shapeInfo[QStringLiteral("innerRadius")] = innerRadius;
            shapeInfo[QStringLiteral("startAngle")] = startAngle;
            shapeInfo[QStringLiteral("endAngle")] = endAngle;
        } else if (count >= 3) {
            const double outerRadius = haversineDistance(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0),
                pointLat(previewPoints, 1), pointLng(previewPoints, 1));
            const double innerRadius = haversineDistance(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0),
                pointLat(previewPoints, 2), pointLng(previewPoints, 2));
            result.geoJson = m_shapeGenerator->generateRing(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0), outerRadius, innerRadius, segments);
            shapeInfo[QStringLiteral("outerRadius")] = outerRadius;
            shapeInfo[QStringLiteral("innerRadius")] = innerRadius;
        } else if (count >= 2) {
            result.geoJson = m_shapeGenerator->circleFromCenterEdge(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0),
                pointLat(previewPoints, 1), pointLng(previewPoints, 1), segments);
            shapeInfo[QStringLiteral("outerRadius")] = haversineDistance(
                pointLat(previewPoints, 0), pointLng(previewPoints, 0),
                pointLat(previewPoints, 1), pointLng(previewPoints, 1));
        }
        break;
    }

    result.shapeInfo = shapeInfo;
    result.annotationPoints = extractAnnotationPoints(result.geoJson);
    return result;
}

QVariantList AirspaceGeometryConverter::extractAnnotationPoints(const QJsonObject& geoJson)
{
    QVariantList result;
    if (!geoJson.contains(QStringLiteral("geometry"))) {
        return result;
    }

    const QJsonObject geometry = geoJson.value(QStringLiteral("geometry")).toObject();
    const QString geometryType = geometry.value(QStringLiteral("type")).toString();
    const QJsonArray coordinates = geometry.value(QStringLiteral("coordinates")).toArray();

    const auto appendPoints = [&result](const QJsonArray& points) {
        for (const QJsonValue& value : points) {
            const QJsonArray point = value.toArray();
            if (point.size() >= 2) {
                result.append(QVariant(QVariantList{ point[1].toDouble(), point[0].toDouble() }));
            }
        }
    };

    if (geometryType == QLatin1String("Polygon")) {
        if (!coordinates.isEmpty()) {
            appendPoints(coordinates[0].toArray());
        }
    } else if (geometryType == QLatin1String("LineString")) {
        appendPoints(coordinates);
    }

    return result;
}

double AirspaceGeometryConverter::haversineDistance(double lat1, double lng1, double lat2, double lng2)
{
    constexpr double earthRadius = 6371000.0;
    const double dLat = qDegreesToRadians(lat2 - lat1);
    const double dLng = qDegreesToRadians(lng2 - lng1);
    const double a = std::sin(dLat / 2.0) * std::sin(dLat / 2.0)
        + std::cos(qDegreesToRadians(lat1)) * std::cos(qDegreesToRadians(lat2))
        * std::sin(dLng / 2.0) * std::sin(dLng / 2.0);
    return earthRadius * 2.0 * std::atan2(std::sqrt(a), std::sqrt(1.0 - a));
}

double AirspaceGeometryConverter::bearingTo(double lat1, double lng1, double lat2, double lng2)
{
    const double dLng = qDegreesToRadians(lng2 - lng1);
    const double y = std::sin(dLng) * std::cos(qDegreesToRadians(lat2));
    const double x = std::cos(qDegreesToRadians(lat1)) * std::sin(qDegreesToRadians(lat2))
        - std::sin(qDegreesToRadians(lat1)) * std::cos(qDegreesToRadians(lat2)) * std::cos(dLng);
    return std::fmod(qRadiansToDegrees(std::atan2(y, x)) + 360.0, 360.0);
}