#include "ShapeGenerator.h"
#include <QtMath>
#include <QJsonDocument>

ShapeGenerator::ShapeGenerator(QObject* parent)
    : QObject(parent)
{
}

// ============================================================================
// 辅助：Haversine 正/反算
// ============================================================================

ShapeGenerator::GeoPoint ShapeGenerator::destinationPoint(
    double lat, double lng, double bearingDeg, double distanceM) const
{
    double latR  = qDegreesToRadians(lat);
    double lngR  = qDegreesToRadians(lng);
    double brngR = qDegreesToRadians(bearingDeg);
    double d     = distanceM / EARTH_RADIUS;

    double newLatR = std::asin(std::sin(latR) * std::cos(d) +
                               std::cos(latR) * std::sin(d) * std::cos(brngR));
    double newLngR = lngR + std::atan2(std::sin(brngR) * std::sin(d) * std::cos(latR),
                                        std::cos(d) - std::sin(latR) * std::sin(newLatR));

    return { qRadiansToDegrees(newLatR), qRadiansToDegrees(newLngR) };
}

double ShapeGenerator::distanceBetween(double lat1, double lng1,
                                        double lat2, double lng2) const
{
    double dLat = qDegreesToRadians(lat2 - lat1);
    double dLng = qDegreesToRadians(lng2 - lng1);
    double a = std::sin(dLat / 2) * std::sin(dLat / 2) +
               std::cos(qDegreesToRadians(lat1)) * std::cos(qDegreesToRadians(lat2)) *
               std::sin(dLng / 2) * std::sin(dLng / 2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    return EARTH_RADIUS * c;
}

double ShapeGenerator::bearingBetween(double lat1, double lng1,
                                       double lat2, double lng2) const
{
    double dLng = qDegreesToRadians(lng2 - lng1);
    double lat1R = qDegreesToRadians(lat1);
    double lat2R = qDegreesToRadians(lat2);

    double y = std::sin(dLng) * std::cos(lat2R);
    double x = std::cos(lat1R) * std::sin(lat2R) -
               std::sin(lat1R) * std::cos(lat2R) * std::cos(dLng);
    return std::fmod(qRadiansToDegrees(std::atan2(y, x)) + 360.0, 360.0);
}

QJsonArray ShapeGenerator::arcCoordinates(double centerLat, double centerLng,
                                           double radiusM,
                                           double startDeg, double endDeg,
                                           int segments) const
{
    QJsonArray coords;
    double step = (endDeg - startDeg) / segments;
    for (int i = 0; i <= segments; ++i) {
        double angle = startDeg + step * i;
        auto pt = destinationPoint(centerLat, centerLng, angle, radiusM);
        coords.append(QJsonArray{ pt.lng, pt.lat });  // GeoJSON: [lng, lat]
    }
    return coords;
}

QJsonObject ShapeGenerator::wrapFeature(const QJsonObject& geometry,
                                         const QJsonObject& properties) const
{
    return QJsonObject{
        { "type", "Feature" },
        { "geometry", geometry },
        { "properties", properties }
    };
}

// ============================================================================
// 形状生成
// ============================================================================

QJsonObject ShapeGenerator::generateRectangle(double centerLat, double centerLng,
                                               double widthM, double heightM,
                                               double rotationDeg) const
{
    // 4 个角点 (从中心出发，先算未旋转时的偏移，再绕中心旋转)
    double halfW = widthM  / 2.0;
    double halfH = heightM / 2.0;

    // 角点方位角 + 距离
    struct Corner { double bearingBase; double dist; };
    double diag = std::sqrt(halfW * halfW + halfH * halfH);
    double angleBase = qRadiansToDegrees(std::atan2(halfW, halfH)); // 右上角的方位角

    Corner corners[4] = {
        { angleBase,         diag },  // 右上
        { 180 - angleBase,   diag },  // 右下
        { 180 + angleBase,   diag },  // 左下
        { 360 - angleBase,   diag },  // 左上
    };

    QJsonArray ring;
    for (auto& c : corners) {
        double bearing = std::fmod(c.bearingBase + rotationDeg + 360.0, 360.0);
        auto pt = destinationPoint(centerLat, centerLng, bearing, c.dist);
        ring.append(QJsonArray{ pt.lng, pt.lat });
    }
    // 闭合
    ring.append(ring[0]);

    QJsonObject geometry{
        { "type", "Polygon" },
        { "coordinates", QJsonArray{ ring } }
    };
    return wrapFeature(geometry);
}

QJsonObject ShapeGenerator::generateSquare(double centerLat, double centerLng,
                                            double sizeM, double rotationDeg) const
{
    return generateRectangle(centerLat, centerLng, sizeM, sizeM, rotationDeg);
}

QJsonObject ShapeGenerator::generateCircle(double centerLat, double centerLng,
                                            double radiusM, int segments) const
{
    QJsonArray ring = arcCoordinates(centerLat, centerLng, radiusM, 0, 360, segments);
    // 确保闭合
    if (ring.size() > 1) {
        ring[ring.size() - 1] = ring[0];
    }

    QJsonObject geometry{
        { "type", "Polygon" },
        { "coordinates", QJsonArray{ ring } }
    };
    return wrapFeature(geometry);
}

QJsonObject ShapeGenerator::generatePolygon(const QVariantList& points) const
{
    QJsonArray ring;
    for (const auto& pt : points) {
        auto list = pt.toList();
        if (list.size() >= 2) {
            double lat = list[0].toDouble();
            double lng = list[1].toDouble();
            ring.append(QJsonArray{ lng, lat });
        }
    }
    // 闭合
    if (ring.size() > 0) {
        ring.append(ring[0]);
    }

    QJsonObject geometry{
        { "type", "Polygon" },
        { "coordinates", QJsonArray{ ring } }
    };
    return wrapFeature(geometry);
}

QJsonObject ShapeGenerator::generateBoundary(const QVariantList& points) const
{
    QJsonArray coords;
    for (const auto& pt : points) {
        auto list = pt.toList();
        if (list.size() >= 2) {
            double lat = list[0].toDouble();
            double lng = list[1].toDouble();
            coords.append(QJsonArray{ lng, lat });
        }
    }

    QJsonObject geometry{
        { "type", "LineString" },
        { "coordinates", coords }
    };
    return wrapFeature(geometry);
}

QJsonObject ShapeGenerator::generateRing(double centerLat, double centerLng,
                                          double outerRadiusM, double innerRadiusM,
                                          int segments) const
{
    QJsonArray outerRing = arcCoordinates(centerLat, centerLng, outerRadiusM, 0, 360, segments);
    if (outerRing.size() > 1) outerRing[outerRing.size() - 1] = outerRing[0];

    // 内环方向相反 (逆时针 = 360→0)
    QJsonArray innerRing = arcCoordinates(centerLat, centerLng, innerRadiusM, 360, 0, segments);
    if (innerRing.size() > 1) innerRing[innerRing.size() - 1] = innerRing[0];

    QJsonObject geometry{
        { "type", "Polygon" },
        { "coordinates", QJsonArray{ outerRing, innerRing } }
    };
    return wrapFeature(geometry);
}

QJsonObject ShapeGenerator::generateArc(double centerLat, double centerLng,
                                         double radiusM,
                                         double startAngleDeg, double endAngleDeg,
                                         int segments) const
{
    QJsonArray coords = arcCoordinates(centerLat, centerLng, radiusM,
                                        startAngleDeg, endAngleDeg, segments);

    QJsonObject geometry{
        { "type", "LineString" },
        { "coordinates", coords }
    };
    return wrapFeature(geometry);
}

QJsonObject ShapeGenerator::generateSector(double centerLat, double centerLng,
                                            double radiusM,
                                            double startAngleDeg, double endAngleDeg,
                                            int segments) const
{
    QJsonArray ring;
    // 起点 = 中心
    ring.append(QJsonArray{ centerLng, centerLat });

    // 弧线坐标
    QJsonArray arc = arcCoordinates(centerLat, centerLng, radiusM,
                                     startAngleDeg, endAngleDeg, segments);
    for (const auto& pt : arc) ring.append(pt);

    // 闭合回中心
    ring.append(QJsonArray{ centerLng, centerLat });

    QJsonObject geometry{
        { "type", "Polygon" },
        { "coordinates", QJsonArray{ ring } }
    };
    return wrapFeature(geometry);
}

QJsonObject ShapeGenerator::generateSectorRing(double centerLat, double centerLng,
                                                double outerRadiusM, double innerRadiusM,
                                                double startAngleDeg, double endAngleDeg,
                                                int segments) const
{
    // 外弧 (顺时针 start→end)
    QJsonArray outerArc = arcCoordinates(centerLat, centerLng, outerRadiusM,
                                          startAngleDeg, endAngleDeg, segments);
    // 内弧 (逆时针 end→start，构成闭合环)
    QJsonArray innerArc = arcCoordinates(centerLat, centerLng, innerRadiusM,
                                          endAngleDeg, startAngleDeg, segments);

    QJsonArray outerRing;
    for (const auto& pt : outerArc) outerRing.append(pt);
    for (const auto& pt : innerArc) outerRing.append(pt);
    // 闭合
    outerRing.append(outerArc[0]);

    QJsonObject geometry{
        { "type", "Polygon" },
        { "coordinates", QJsonArray{ outerRing } }
    };
    return wrapFeature(geometry);
}

QJsonObject ShapeGenerator::rectangleFromCorners(double lat1, double lng1,
                                                  double lat2, double lng2) const
{
    QJsonArray ring;
    ring.append(QJsonArray{ lng1, lat1 });
    ring.append(QJsonArray{ lng2, lat1 });
    ring.append(QJsonArray{ lng2, lat2 });
    ring.append(QJsonArray{ lng1, lat2 });
    ring.append(QJsonArray{ lng1, lat1 });

    QJsonObject geometry{
        { "type", "Polygon" },
        { "coordinates", QJsonArray{ ring } }
    };
    return wrapFeature(geometry);
}

QJsonObject ShapeGenerator::circleFromCenterEdge(double cLat, double cLng,
                                                  double eLat, double eLng,
                                                  int segments) const
{
    double radius = distanceBetween(cLat, cLng, eLat, eLng);
    return generateCircle(cLat, cLng, radius, segments);
}

// ============================================================================
// 高频预览用，返回纯坐标点集避免 JSON 操作
// 返回结果形式: [[lat, lng], [lat, lng], ...]
// ============================================================================

QVariantList ShapeGenerator::computeRectanglePoints(double centerLat, double centerLng, 
                                                    double widthM, double heightM, 
                                                    double rotationDeg) const
{
    double halfW = widthM  / 2.0;
    double halfH = heightM / 2.0;

    struct Corner { double bearingBase; double dist; };
    double diag = std::sqrt(halfW * halfW + halfH * halfH);
    double angleBase = qRadiansToDegrees(std::atan2(halfW, halfH)); 

    Corner corners[4] = {
        { angleBase,         diag },  
        { 180 - angleBase,   diag },  
        { 180 + angleBase,   diag },  
        { 360 - angleBase,   diag },  
    };

    QVariantList ring;
    for (auto& c : corners) {
        double bearing = std::fmod(c.bearingBase + rotationDeg + 360.0, 360.0);
        auto pt = destinationPoint(centerLat, centerLng, bearing, c.dist);
        ring.append(QVariant::fromValue(QVariantList{ pt.lat, pt.lng }));
    }
    if (!ring.isEmpty()) {
        ring.append(ring.first()); // 闭合
    }
    return ring;
}

QVariantList ShapeGenerator::computeCirclePoints(double centerLat, double centerLng, 
                                                 double radiusM, int segments) const
{
    QVariantList ring;
    double step = 360.0 / segments;
    for (int i = 0; i <= segments; ++i) {
        double angle = step * i;
        auto pt = destinationPoint(centerLat, centerLng, angle, radiusM);
        ring.append(QVariant::fromValue(QVariantList{ pt.lat, pt.lng }));
    }
    return ring;
}

QVariantList ShapeGenerator::computeLinePoints(const QVariantList& points) const
{
    return points;
}

QVariantList ShapeGenerator::computeSquarePoints(double centerLat, double centerLng, 
                                                 double sizeM, double rotationDeg) const
{
    return computeRectanglePoints(centerLat, centerLng, sizeM, sizeM, rotationDeg);
}
