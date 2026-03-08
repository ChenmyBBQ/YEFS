#include "MapLibreEngine.h"
#include "MessageBus.h"
#include <QMapLibre/Map>
#include <QDebug>
#include <QMetaObject>
#include <QJsonArray>
#include <QJsonDocument>

namespace YEFS {

// ---------- 内部辅助函数 ----------

static QMapLibre::Map* getNativeMap(QObject* mapItem) {
    if (!mapItem) return nullptr;
    QObject* result = nullptr;
    QMetaObject::invokeMethod(mapItem, "getNativeMap",
                              Qt::DirectConnection,
                              Q_RETURN_ARG(QObject*, result));
    return qobject_cast<QMapLibre::Map*>(result);
}

static QString detectGeometryType(const QJsonObject& geoJson) {
    QString type = geoJson["type"].toString();
    if (type == QLatin1String("Feature"))
        return geoJson["geometry"].toObject()["type"].toString();
    if (type == QLatin1String("FeatureCollection")) {
        auto features = geoJson["features"].toArray();
        if (!features.isEmpty())
            return features[0].toObject()["geometry"].toObject()["type"].toString();
    }
    return type;
}

MapLibreEngine* MapLibreEngine::s_instance = nullptr;

MapLibreEngine::MapLibreEngine(QObject* parent)
    : IMapEngine(parent)
{
    // 添加默认样式
    m_styles["OSM Demo"] = "https://demotiles.maplibre.org/style.json";
    m_styles["MapTiler Streets"] = "https://api.maptiler.com/maps/streets/style.json";
    m_styles["MapTiler Satellite"] = "https://api.maptiler.com/maps/satellite/style.json";

    m_currentStyle = m_styles["OSM Demo"];
}

MapLibreEngine* MapLibreEngine::instance()
{
    if (!s_instance) {
        s_instance = new MapLibreEngine();
    }
    return s_instance;
}

MapLibreEngine* MapLibreEngine::create(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    return instance();
}

void MapLibreEngine::destroy()
{
    delete s_instance;
    s_instance = nullptr;
}

QUrl MapLibreEngine::qmlComponent() const
{
    return QUrl(QStringLiteral("qrc:/YEFS/qml/Map/MapLibreView.qml"));
}

void MapLibreEngine::setMapItem(QObject* mapItem)
{
    m_mapItem = mapItem;
    if (m_mapItem) {
        qDebug() << "[MapLibreEngine] Map item connected";
    }
}

void MapLibreEngine::setCenter(double latitude, double longitude)
{
    if (m_mapItem) {
        QMetaObject::invokeMethod(m_mapItem, "setCenter",
                                  Q_ARG(double, latitude),
                                  Q_ARG(double, longitude));
    }
    m_latitude = latitude;
    m_longitude = longitude;
}

void MapLibreEngine::setZoom(double zoom)
{
    if (m_mapItem) {
        QMetaObject::invokeMethod(m_mapItem, "setZoom",
                                  Q_ARG(double, zoom));
    }
    m_zoom = zoom;
}

void MapLibreEngine::setPitch(double pitch)
{
    if (m_mapItem) {
        QMetaObject::invokeMethod(m_mapItem, "setPitch",
                                  Q_ARG(double, pitch));
    }
    m_pitch = pitch;
}

void MapLibreEngine::setBearing(double bearing)
{
    if (m_mapItem) {
        QMetaObject::invokeMethod(m_mapItem, "setBearing",
                                  Q_ARG(double, bearing));
    }
    m_bearing = bearing;
}

void MapLibreEngine::flyTo(double latitude, double longitude, double zoom, int durationMs)
{
    if (m_mapItem) {
        QMetaObject::invokeMethod(m_mapItem, "flyTo",
                                  Q_ARG(double, latitude),
                                  Q_ARG(double, longitude),
                                  Q_ARG(double, zoom),
                                  Q_ARG(int, durationMs));
    }
}

void MapLibreEngine::addGeoJSONLayer(const QString& layerId,
                                      const QJsonObject& geoJson,
                                      const QVariantMap& style)
{
    m_layers[layerId] = geoJson;

    auto* map = getNativeMap(m_mapItem);
    if (!map) {
        qWarning() << "[MapLibreEngine] addGeoJSONLayer: native map not ready";
        return;
    }

    // 添加 GeoJSON 数据源，data 必须是 JSON 字节（QByteArray），不能是 QVariantMap
    QVariantMap sourceParams;
    sourceParams[QStringLiteral("type")] = QStringLiteral("geojson");
    sourceParams[QStringLiteral("data")] = QJsonDocument(geoJson).toJson(QJsonDocument::Compact);
    map->addSource(layerId + QStringLiteral("-source"), sourceParams);

    // 根据几何类型分别添加填充层和线框层
    const QString geomType = detectGeometryType(geoJson);
    if (geomType == QLatin1String("Polygon") || geomType == QLatin1String("MultiPolygon")) {
        QVariantMap fillParams;
        fillParams[QStringLiteral("type")]   = QStringLiteral("fill");
        fillParams[QStringLiteral("source")] = layerId + QStringLiteral("-source");
        map->addLayer(layerId + QStringLiteral("-fill"), fillParams);
        map->setPaintProperty(layerId + QStringLiteral("-fill"), QStringLiteral("fill-color"),
                              style.value(QStringLiteral("fill-color"), QStringLiteral("#3388ff")));
        map->setPaintProperty(layerId + QStringLiteral("-fill"), QStringLiteral("fill-opacity"),
                              style.value(QStringLiteral("fill-opacity"), 0.3));
    }

    // 所有几何类型都添加边界线层
    QVariantMap lineParams;
    lineParams[QStringLiteral("type")]   = QStringLiteral("line");
    lineParams[QStringLiteral("source")] = layerId + QStringLiteral("-source");
    map->addLayer(layerId + QStringLiteral("-line"), lineParams);
    map->setPaintProperty(layerId + QStringLiteral("-line"), QStringLiteral("line-color"),
                          style.value(QStringLiteral("line-color"), QStringLiteral("#3388ff")));
    map->setPaintProperty(layerId + QStringLiteral("-line"), QStringLiteral("line-width"),
                          style.value(QStringLiteral("line-width"), 2));
    if (style.contains(QStringLiteral("line-dasharray")))
        map->setPaintProperty(layerId + QStringLiteral("-line"), QStringLiteral("line-dasharray"),
                              style[QStringLiteral("line-dasharray")]);

    emit layerAdded(layerId);
    MessageBus::instance()->publish(Topics::MAP_LAYER_ADDED, layerId);
}

void MapLibreEngine::removeLayer(const QString& layerId)
{
    m_layers.remove(layerId);

    auto* map = getNativeMap(m_mapItem);
    if (map) {
        if (map->layerExists(layerId + QStringLiteral("-fill")))
            map->removeLayer(layerId + QStringLiteral("-fill"));
        if (map->layerExists(layerId + QStringLiteral("-line")))
            map->removeLayer(layerId + QStringLiteral("-line"));
        if (map->sourceExists(layerId + QStringLiteral("-source")))
            map->removeSource(layerId + QStringLiteral("-source"));
    }

    emit layerRemoved(layerId);
    MessageBus::instance()->publish(Topics::MAP_LAYER_REMOVED, layerId);
}

void MapLibreEngine::setLayerVisibility(const QString& layerId, bool visible)
{
    auto* map = getNativeMap(m_mapItem);
    if (!map) return;
    const QString vis = visible ? QStringLiteral("visible") : QStringLiteral("none");
    if (map->layerExists(layerId + QStringLiteral("-fill")))
        map->setLayoutProperty(layerId + QStringLiteral("-fill"), QStringLiteral("visibility"), vis);
    if (map->layerExists(layerId + QStringLiteral("-line")))
        map->setLayoutProperty(layerId + QStringLiteral("-line"), QStringLiteral("visibility"), vis);
}

void MapLibreEngine::updateLayerData(const QString& layerId, const QJsonObject& geoJson)
{
    m_layers[layerId] = geoJson;

    auto* map = getNativeMap(m_mapItem);
    if (!map) return;
    if (map->sourceExists(layerId + QStringLiteral("-source"))) {
        QVariantMap params;
        params[QStringLiteral("type")] = QStringLiteral("geojson");
        params[QStringLiteral("data")] = QJsonDocument(geoJson).toJson(QJsonDocument::Compact);
        map->updateSource(layerId + QStringLiteral("-source"), params);
    } else {
        // 源不存在时退回到完整添加
        addGeoJSONLayer(layerId, geoJson, {});
    }
}

void MapLibreEngine::setStyle(const QString& styleUrl)
{
    m_currentStyle = styleUrl;

    if (m_mapItem) {
        QMetaObject::invokeMethod(m_mapItem, "setStyle",
                                  Q_ARG(QString, styleUrl));
    }

    emit styleChanged(styleUrl);
    MessageBus::instance()->publish(Topics::MAP_STYLE_CHANGED, styleUrl);
}

QPointF MapLibreEngine::coordinateToScreen(double latitude, double longitude) const
{
    if (m_mapItem) {
        QPointF result;
        QMetaObject::invokeMethod(m_mapItem, "coordinateToScreen",
                                  Q_RETURN_ARG(QPointF, result),
                                  Q_ARG(double, latitude),
                                  Q_ARG(double, longitude));
        return result;
    }
    return QPointF();
}

QGeoCoordinate MapLibreEngine::screenToCoordinate(double x, double y) const
{
    if (m_mapItem) {
        QGeoCoordinate result;
        QMetaObject::invokeMethod(m_mapItem, "screenToCoordinate",
                                  Q_RETURN_ARG(QGeoCoordinate, result),
                                  Q_ARG(double, x),
                                  Q_ARG(double, y));
        return result;
    }
    return QGeoCoordinate();
}

QStringList MapLibreEngine::availableStyles() const
{
    return m_styles.keys();
}

void MapLibreEngine::addStyle(const QString& name, const QString& url)
{
    m_styles[name] = url;
}

void MapLibreEngine::onMapReady()
{
    m_ready = true;
    emit readyChanged(true);
    MessageBus::instance()->publish(Topics::MAP_READY, true);
    qDebug() << "[MapLibreEngine] Map is ready";
}

void MapLibreEngine::onCenterChanged(double latitude, double longitude)
{
    m_latitude = latitude;
    m_longitude = longitude;
    emit centerChanged(latitude, longitude);
    
    QVariantMap data;
    data["latitude"] = latitude;
    data["longitude"] = longitude;
    MessageBus::instance()->publish(Topics::MAP_CENTER_CHANGED, data);
}

void MapLibreEngine::onZoomChanged(double zoom)
{
    m_zoom = zoom;
    emit zoomChanged(zoom);
    MessageBus::instance()->publish(Topics::MAP_ZOOM_CHANGED, zoom);
}

void MapLibreEngine::onPitchChanged(double pitch)
{
    m_pitch = pitch;
    emit pitchChanged(pitch);
}

void MapLibreEngine::onBearingChanged(double bearing)
{
    m_bearing = bearing;
    emit bearingChanged(bearing);
}

void MapLibreEngine::onMapClicked(double latitude, double longitude)
{
    emit mapClicked(latitude, longitude);

    QVariantMap data;
    data["latitude"] = latitude;
    data["longitude"] = longitude;
    MessageBus::instance()->publish(Topics::MAP_CLICKED, data);
}

} // namespace YEFS
