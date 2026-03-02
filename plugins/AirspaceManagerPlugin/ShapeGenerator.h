#ifndef SHAPEGENERATOR_H
#define SHAPEGENERATOR_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantList>

/**
 * @brief 形状→GeoJSON 生成器
 *
 * 将各种空域形状参数转为 GeoJSON Feature 对象。
 * MapLibre 不支持曲线几何体，圆/弧/扇/环全部离散为多边形/折线。
 */
class ShapeGenerator : public QObject
{
    Q_OBJECT

public:
    explicit ShapeGenerator(QObject* parent = nullptr);

    /// 矩形 (center + 宽高米 + 旋转角度)
    Q_INVOKABLE QJsonObject generateRectangle(double centerLat, double centerLng,
                                               double widthM, double heightM,
                                               double rotationDeg = 0) const;

    /// 正方形 (center + 边长米 + 旋转角度)
    Q_INVOKABLE QJsonObject generateSquare(double centerLat, double centerLng,
                                            double sizeM, double rotationDeg = 0) const;

    /// 圆形 (center + 半径米, segments 离散段数)
    Q_INVOKABLE QJsonObject generateCircle(double centerLat, double centerLng,
                                            double radiusM, int segments = 64) const;

    /// 多边形 (坐标列表 [[lat,lng], ...])
    Q_INVOKABLE QJsonObject generatePolygon(const QVariantList& points) const;

    /// 边界线 / 不闭合连线
    Q_INVOKABLE QJsonObject generateBoundary(const QVariantList& points) const;

    /// 圆环 (center + 内外半径)
    Q_INVOKABLE QJsonObject generateRing(double centerLat, double centerLng,
                                          double outerRadiusM, double innerRadiusM,
                                          int segments = 64) const;

    /// 圆弧 (LineString)
    Q_INVOKABLE QJsonObject generateArc(double centerLat, double centerLng,
                                         double radiusM,
                                         double startAngleDeg, double endAngleDeg,
                                         int segments = 64) const;

    /// 扇形 (Polygon)
    Q_INVOKABLE QJsonObject generateSector(double centerLat, double centerLng,
                                            double radiusM,
                                            double startAngleDeg, double endAngleDeg,
                                            int segments = 64) const;

    /// 扇环形 (Polygon with hole)
    Q_INVOKABLE QJsonObject generateSectorRing(double centerLat, double centerLng,
                                                double outerRadiusM, double innerRadiusM,
                                                double startAngleDeg, double endAngleDeg,
                                                int segments = 64) const;

    /// 从两个对角点构建矩形
    Q_INVOKABLE QJsonObject rectangleFromCorners(double lat1, double lng1,
                                                  double lat2, double lng2) const;

    /// 从中心 + 边缘点构建圆形
    Q_INVOKABLE QJsonObject circleFromCenterEdge(double cLat, double cLng,
                                                  double eLat, double eLng,
                                                  int segments = 64) const;

private:
    // 经纬度偏移工具 (Haversine 正/反算)
    struct GeoPoint { double lat; double lng; };

    /// 从 (lat, lng) 沿方位角 bearingDeg 偏移 distanceM 米
    GeoPoint destinationPoint(double lat, double lng,
                              double bearingDeg, double distanceM) const;

    /// 两点间距离 (米)
    double distanceBetween(double lat1, double lng1,
                           double lat2, double lng2) const;

    /// 两点间方位角 (度)
    double bearingBetween(double lat1, double lng1,
                          double lat2, double lng2) const;

    /// 生成圆弧上的坐标数组
    QJsonArray arcCoordinates(double centerLat, double centerLng,
                              double radiusM,
                              double startDeg, double endDeg,
                              int segments) const;

    /// 包装为 GeoJSON Feature
    QJsonObject wrapFeature(const QJsonObject& geometry,
                            const QJsonObject& properties = {}) const;

    static constexpr double EARTH_RADIUS = 6371000.0; // 米
};

#endif // SHAPEGENERATOR_H
