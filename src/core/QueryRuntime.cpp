#include "QueryRuntime.h"

#include "MapLibreEngine.h"
#include "MessageBus.h"

#include <QGeoCoordinate>
#include <QJsonArray>
#include <QJsonObject>
#include <QSet>
#include <QtMath>

namespace YEFS {

namespace {

struct QueryFeatureResult {
    QVariantMap data;
    double distance = 0.0;
};

QPointF toScreenPoint(const QJsonArray& coordinate)
{
    if (coordinate.size() < 2) {
        return {};
    }

    return MapLibreEngine::instance()->coordinateToScreen(coordinate.at(1).toDouble(),
                                                          coordinate.at(0).toDouble());
}

double distanceSquared(const QPointF& left, const QPointF& right)
{
    const double dx = left.x() - right.x();
    const double dy = left.y() - right.y();
    return dx * dx + dy * dy;
}

double pointToSegmentDistanceSquared(const QPointF& point,
                                     const QPointF& segmentStart,
                                     const QPointF& segmentEnd)
{
    const QPointF segment = segmentEnd - segmentStart;
    const double lengthSquared = segment.x() * segment.x() + segment.y() * segment.y();
    if (qFuzzyIsNull(lengthSquared)) {
        return distanceSquared(point, segmentStart);
    }

    const QPointF fromStart = point - segmentStart;
    const double t = qBound(0.0,
                            QPointF::dotProduct(fromStart, segment) / lengthSquared,
                            1.0);
    const QPointF projection = segmentStart + t * segment;
    return distanceSquared(point, projection);
}

bool pointInRing(const QPointF& point, const QVector<QPointF>& ring)
{
    if (ring.size() < 3) {
        return false;
    }

    bool inside = false;
    for (int i = 0, j = ring.size() - 1; i < ring.size(); j = i++) {
        const QPointF& current = ring.at(i);
        const QPointF& previous = ring.at(j);
        const bool intersects = ((current.y() > point.y()) != (previous.y() > point.y()))
            && (point.x() < (previous.x() - current.x()) * (point.y() - current.y())
                            / (previous.y() - current.y() + 1e-9) + current.x());
        if (intersects) {
            inside = !inside;
        }
    }

    return inside;
}

double ringEdgeDistanceSquared(const QPointF& point, const QVector<QPointF>& ring)
{
    if (ring.size() < 2) {
        return std::numeric_limits<double>::max();
    }

    double minDistanceSquared = std::numeric_limits<double>::max();
    for (int i = 1; i < ring.size(); ++i) {
        minDistanceSquared = qMin(minDistanceSquared,
                                  pointToSegmentDistanceSquared(point, ring.at(i - 1), ring.at(i)));
    }
    return minDistanceSquared;
}

QVector<QPointF> screenRingFromJson(const QJsonArray& ringArray)
{
    QVector<QPointF> ring;
    ring.reserve(ringArray.size());
    for (const QJsonValue& pointValue : ringArray) {
        ring.append(toScreenPoint(pointValue.toArray()));
    }
    return ring;
}

QVariantMap makeResult(const QString& layerId,
                       const QString& groupId,
                       const QString& businessObjectId,
                       const QString& featureId,
                       const QString& geometryType,
                       const QJsonObject& properties,
                       const QGeoCoordinate& coordinate,
                       double distance)
{
    QVariantMap result;
    result[QStringLiteral("layerId")] = layerId;
    result[QStringLiteral("groupId")] = groupId;
    result[QStringLiteral("businessObjectId")] = businessObjectId;
    result[QStringLiteral("featureId")] = featureId;
    result[QStringLiteral("geometryType")] = geometryType;
    result[QStringLiteral("properties")] = properties.toVariantMap();
    result[QStringLiteral("distancePx")] = distance;

    QVariantMap coordinateMap;
    coordinateMap[QStringLiteral("latitude")] = coordinate.latitude();
    coordinateMap[QStringLiteral("longitude")] = coordinate.longitude();
    result[QStringLiteral("coordinate")] = coordinateMap;
    return result;
}

QString featureIdentifier(const QJsonObject& feature, int fallbackIndex)
{
    const QJsonValue idValue = feature.value(QStringLiteral("id"));
    if (!idValue.isUndefined() && !idValue.isNull()) {
        return idValue.toVariant().toString();
    }

    const QJsonObject properties = feature.value(QStringLiteral("properties")).toObject();
    const QJsonValue propertiesId = properties.value(QStringLiteral("id"));
    if (!propertiesId.isUndefined() && !propertiesId.isNull()) {
        return propertiesId.toVariant().toString();
    }

    return QStringLiteral("feature-%1").arg(fallbackIndex);
}

QJsonArray featureArrayFromGeoJson(const QJsonObject& geoJson)
{
    const QString type = geoJson.value(QStringLiteral("type")).toString();
    if (type == QLatin1String("FeatureCollection")) {
        return geoJson.value(QStringLiteral("features")).toArray();
    }

    if (type == QLatin1String("Feature")) {
        return QJsonArray{ geoJson };
    }

    return QJsonArray{ QJsonObject{
        { QStringLiteral("type"), QStringLiteral("Feature") },
        { QStringLiteral("geometry"), geoJson },
        { QStringLiteral("properties"), QJsonObject{} }
    } };
}

bool pointHitsGeometry(const QPointF& point,
                       const QString& geometryType,
                       const QJsonValue& coordinatesValue,
                       double toleranceSquared,
                       double* outDistanceSquared)
{
    *outDistanceSquared = std::numeric_limits<double>::max();

    if (geometryType == QLatin1String("Point")) {
        const double distance = distanceSquared(point, toScreenPoint(coordinatesValue.toArray()));
        *outDistanceSquared = distance;
        return distance <= toleranceSquared;
    }

    if (geometryType == QLatin1String("MultiPoint")) {
        for (const QJsonValue& pointValue : coordinatesValue.toArray()) {
            const double distance = distanceSquared(point, toScreenPoint(pointValue.toArray()));
            *outDistanceSquared = qMin(*outDistanceSquared, distance);
            if (distance <= toleranceSquared) {
                return true;
            }
        }
        return false;
    }

    if (geometryType == QLatin1String("LineString")) {
        const QJsonArray points = coordinatesValue.toArray();
        for (int i = 1; i < points.size(); ++i) {
            const double distance = pointToSegmentDistanceSquared(point,
                toScreenPoint(points.at(i - 1).toArray()),
                toScreenPoint(points.at(i).toArray()));
            *outDistanceSquared = qMin(*outDistanceSquared, distance);
            if (distance <= toleranceSquared) {
                return true;
            }
        }
        return false;
    }

    if (geometryType == QLatin1String("MultiLineString")) {
        for (const QJsonValue& lineValue : coordinatesValue.toArray()) {
            double distance = std::numeric_limits<double>::max();
            if (pointHitsGeometry(point, QStringLiteral("LineString"), lineValue,
                                  toleranceSquared, &distance)) {
                *outDistanceSquared = qMin(*outDistanceSquared, distance);
                return true;
            }
            *outDistanceSquared = qMin(*outDistanceSquared, distance);
        }
        return false;
    }

    if (geometryType == QLatin1String("Polygon")) {
        const QJsonArray polygon = coordinatesValue.toArray();
        if (polygon.isEmpty()) {
            return false;
        }

        QVector<QPointF> outerRing = screenRingFromJson(polygon.at(0).toArray());
        const bool insideOuter = pointInRing(point, outerRing);
        bool insideHole = false;
        for (int i = 1; i < polygon.size(); ++i) {
            const QVector<QPointF> holeRing = screenRingFromJson(polygon.at(i).toArray());
            insideHole = insideHole || pointInRing(point, holeRing);
            *outDistanceSquared = qMin(*outDistanceSquared, ringEdgeDistanceSquared(point, holeRing));
        }

        *outDistanceSquared = qMin(*outDistanceSquared, ringEdgeDistanceSquared(point, outerRing));
        if (insideOuter && !insideHole) {
            *outDistanceSquared = 0.0;
            return true;
        }

        return *outDistanceSquared <= toleranceSquared;
    }

    if (geometryType == QLatin1String("MultiPolygon")) {
        for (const QJsonValue& polygonValue : coordinatesValue.toArray()) {
            double distance = std::numeric_limits<double>::max();
            if (pointHitsGeometry(point, QStringLiteral("Polygon"), polygonValue,
                                  toleranceSquared, &distance)) {
                *outDistanceSquared = qMin(*outDistanceSquared, distance);
                return true;
            }
            *outDistanceSquared = qMin(*outDistanceSquared, distance);
        }
        return false;
    }

    return false;
}

} // namespace

QueryRuntime* QueryRuntime::s_instance = nullptr;

QueryRuntime::QueryRuntime(QObject* parent)
    : QObject(parent)
{
}

QueryRuntime* QueryRuntime::instance()
{
    if (!s_instance) {
        s_instance = new QueryRuntime();
    }
    return s_instance;
}

QueryRuntime* QueryRuntime::create(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    return instance();
}

void QueryRuntime::destroy()
{
    delete s_instance;
    s_instance = nullptr;
}

QVariantList QueryRuntime::queryFeaturesAtScreenPoint(const QPointF& screenPosition,
                                                      const QVariantList& layerIds,
                                                      const QString& groupId,
                                                      double tolerancePx)
{
    const QGeoCoordinate coordinate = MapLibreEngine::instance()->screenToCoordinate(screenPosition.x(),
                                                                                      screenPosition.y());
    if (!coordinate.isValid()) {
        setLastResults({});
        return {};
    }

    QSet<QString> layerIdFilter;
    for (const QVariant& layerIdValue : layerIds) {
        const QString layerId = layerIdValue.toString().trimmed();
        if (!layerId.isEmpty()) {
            layerIdFilter.insert(layerId);
        }
    }

    QList<QueryFeatureResult> hitResults;
    const double toleranceSquared = tolerancePx * tolerancePx;
    const QList<MapLibreEngine::QueryLayerSnapshot> layers = MapLibreEngine::instance()->queryableLayersSnapshot();
    for (const MapLibreEngine::QueryLayerSnapshot& layer : layers) {
        if (!layer.queryEnabled || !layer.visible) {
            continue;
        }
        if (!groupId.isEmpty() && layer.groupId != groupId) {
            continue;
        }
        if (!layerIdFilter.isEmpty() && !layerIdFilter.contains(layer.layerId)) {
            continue;
        }

        const QJsonArray features = featureArrayFromGeoJson(layer.geoJson);
        for (int featureIndex = 0; featureIndex < features.size(); ++featureIndex) {
            const QJsonObject feature = features.at(featureIndex).toObject();
            const QJsonObject geometry = feature.value(QStringLiteral("geometry")).toObject();
            const QString geometryType = geometry.value(QStringLiteral("type")).toString();
            const QJsonObject properties = feature.value(QStringLiteral("properties")).toObject();

            double distance = std::numeric_limits<double>::max();
            if (!pointHitsGeometry(screenPosition, geometryType,
                                   geometry.value(QStringLiteral("coordinates")),
                                   toleranceSquared, &distance)) {
                continue;
            }

            const QString businessObjectId = properties.value(QStringLiteral("businessObjectId")).toString(
                layer.businessObjectId);
            QueryFeatureResult hit;
            hit.distance = qSqrt(distance);
            hit.data = makeResult(layer.layerId,
                                  layer.groupId,
                                  businessObjectId,
                                  featureIdentifier(feature, featureIndex),
                                  geometryType,
                                  properties,
                                  coordinate,
                                  hit.distance);
            hitResults.append(hit);
        }
    }

    std::sort(hitResults.begin(), hitResults.end(), [](const QueryFeatureResult& left,
                                                       const QueryFeatureResult& right) {
        return left.distance < right.distance;
    });

    QVariantList results;
    results.reserve(hitResults.size());
    for (const QueryFeatureResult& hit : hitResults) {
        results.append(hit.data);
    }

    setLastResults(results);
    return results;
}

QVariantList QueryRuntime::handleTap(const QPointF& screenPosition,
                                     const QVariantList& layerIds,
                                     const QString& groupId)
{
    const QVariantList results = queryFeaturesAtScreenPoint(screenPosition, layerIds, groupId);

    QVariantMap payload;
    payload[QStringLiteral("screenX")] = screenPosition.x();
    payload[QStringLiteral("screenY")] = screenPosition.y();
    payload[QStringLiteral("results")] = results;
    if (!groupId.isEmpty()) {
        payload[QStringLiteral("groupId")] = groupId;
    }

    if (results.isEmpty()) {
        MessageBus::instance()->publish(Topics::MAP_QUERY_EMPTY, payload);
    } else {
        MessageBus::instance()->publish(Topics::MAP_QUERY_RESULT, payload);
    }

    return results;
}

void QueryRuntime::setLastResults(const QVariantList& results)
{
    if (m_lastResults == results) {
        return;
    }

    m_lastResults = results;
    emit lastResultsChanged();
}

} // namespace YEFS