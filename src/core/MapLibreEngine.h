#ifndef YEFS_MAPLIBREENGINE_H
#define YEFS_MAPLIBREENGINE_H

#include "IMapEngine.h"
#include <QQmlEngine>
#include <QHash>
#include <QPointF>

namespace YEFS {

/**
 * @brief MapLibre 地图引擎实现
 * 
 * 封装 MapLibre Native Qt，提供统一的地图引擎接口
 */
class MapLibreEngine : public IMapEngine
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    static MapLibreEngine* instance();
    static MapLibreEngine* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine);

    // IMapEngine 接口实现
    QString name() const override { return QStringLiteral("MapLibre"); }
    EngineType engineType() const override { return Engine25D; }
    bool isReady() const override { return m_ready; }
    QUrl qmlComponent() const override;

    // 相机控制
    Q_INVOKABLE void setCenter(double latitude, double longitude) override;
    Q_INVOKABLE void setZoom(double zoom) override;
    Q_INVOKABLE void setPitch(double pitch) override;
    Q_INVOKABLE void setBearing(double bearing) override;
    Q_INVOKABLE void flyTo(double latitude, double longitude, 
                           double zoom = -1, int durationMs = 1000) override;

    // 图层管理
    Q_INVOKABLE void addGeoJSONLayer(const QString& layerId, 
                                      const QJsonObject& geoJson,
                                      const QVariantMap& style = {}) override;
    Q_INVOKABLE void removeLayer(const QString& layerId) override;
    Q_INVOKABLE void setLayerVisibility(const QString& layerId, bool visible) override;
    Q_INVOKABLE void updateLayerData(const QString& layerId, 
                                      const QJsonObject& geoJson) override;

    // 样式管理
    Q_INVOKABLE void setStyle(const QString& styleUrl) override;
    Q_INVOKABLE QString currentStyle() const override { return m_currentStyle; }

    // 坐标转换 (需要 QML 端配合)
    Q_INVOKABLE QPointF coordinateToScreen(double latitude, double longitude) const override;
    Q_INVOKABLE QGeoCoordinate screenToCoordinate(double x, double y) const override;

    // MapLibre 特有功能
    Q_INVOKABLE void setMapItem(QObject* mapItem);
    Q_INVOKABLE QStringList availableStyles() const;
    Q_INVOKABLE void addStyle(const QString& name, const QString& url);

    // 当前相机状态
    Q_PROPERTY(double latitude READ latitude NOTIFY centerChanged)
    Q_PROPERTY(double longitude READ longitude NOTIFY centerChanged)
    Q_PROPERTY(double zoom READ zoom NOTIFY zoomChanged)
    Q_PROPERTY(double pitch READ pitch NOTIFY pitchChanged)
    Q_PROPERTY(double bearing READ bearing NOTIFY bearingChanged)

    double latitude() const { return m_latitude; }
    double longitude() const { return m_longitude; }
    double zoom() const { return m_zoom; }
    double pitch() const { return m_pitch; }
    double bearing() const { return m_bearing; }

signals:
    void centerChanged(double latitude, double longitude);
    void zoomChanged(double zoom);
    void pitchChanged(double pitch);
    void bearingChanged(double bearing);

public slots:
    // 由 QML 调用，同步状态
    void onMapReady();
    void onCenterChanged(double latitude, double longitude);
    void onZoomChanged(double zoom);
    void onPitchChanged(double pitch);
    void onBearingChanged(double bearing);
    void onMapClicked(double latitude, double longitude);

private:
    explicit MapLibreEngine(QObject* parent = nullptr);
    ~MapLibreEngine() override = default;

    static MapLibreEngine* s_instance;

    bool m_ready = false;
    QString m_currentStyle;
    QHash<QString, QString> m_styles;  // name -> url
    QObject* m_mapItem = nullptr;

    // 当前相机状态
    double m_latitude = 39.9042;
    double m_longitude = 116.4074;
    double m_zoom = 2.0;
    double m_pitch = 0.0;
    double m_bearing = 0.0;

    // 图层跟踪
    QHash<QString, QJsonObject> m_layers;
};

} // namespace YEFS

#endif // YEFS_MAPLIBREENGINE_H
