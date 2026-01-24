#include "MapLibreEngine.h"
#include "MessageBus.h"
#include <QDebug>
#include <QMetaObject>

namespace YEFS {

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

    if (m_mapItem) {
        QMetaObject::invokeMethod(m_mapItem, "addGeoJSONLayer",
                                  Q_ARG(QString, layerId),
                                  Q_ARG(QJsonObject, geoJson),
                                  Q_ARG(QVariantMap, style));
    }

    emit layerAdded(layerId);
    MessageBus::instance()->publish(Topics::MAP_LAYER_ADDED, layerId);
}

void MapLibreEngine::removeLayer(const QString& layerId)
{
    m_layers.remove(layerId);

    if (m_mapItem) {
        QMetaObject::invokeMethod(m_mapItem, "removeLayer",
                                  Q_ARG(QString, layerId));
    }

    emit layerRemoved(layerId);
    MessageBus::instance()->publish(Topics::MAP_LAYER_REMOVED, layerId);
}

void MapLibreEngine::setLayerVisibility(const QString& layerId, bool visible)
{
    if (m_mapItem) {
        QMetaObject::invokeMethod(m_mapItem, "setLayerVisibility",
                                  Q_ARG(QString, layerId),
                                  Q_ARG(bool, visible));
    }
}

void MapLibreEngine::updateLayerData(const QString& layerId, const QJsonObject& geoJson)
{
    m_layers[layerId] = geoJson;

    if (m_mapItem) {
        QMetaObject::invokeMethod(m_mapItem, "updateLayerData",
                                  Q_ARG(QString, layerId),
                                  Q_ARG(QJsonObject, geoJson));
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
