#ifndef YEFS_IMAPENGINE_H
#define YEFS_IMAPENGINE_H

#include <QObject>
#include <QUrl>
#include <QGeoCoordinate>
#include <QJsonObject>
#include <QVariantMap>

namespace YEFS {

/**
 * @brief 地图引擎接口
 * 
 * 定义了地图引擎的标准接口，所有地图引擎实现（MapLibre、Cesium等）都必须实现此接口
 */
class IMapEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(EngineType engineType READ engineType CONSTANT)
    Q_PROPERTY(bool ready READ isReady NOTIFY readyChanged)
    Q_PROPERTY(QUrl qmlComponent READ qmlComponent CONSTANT)

public:
    enum EngineType {
        Engine2D,       // 2D 平面地图
        Engine25D,      // 2.5D 倾斜地图 (MapLibre)
        Engine3D        // 3D 地球 (Cesium, osgEarth)
    };
    Q_ENUM(EngineType)

    explicit IMapEngine(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IMapEngine() = default;

    // 引擎信息
    virtual QString name() const = 0;
    virtual EngineType engineType() const = 0;
    virtual bool isReady() const = 0;

    // QML 组件
    virtual QUrl qmlComponent() const = 0;

    // 相机控制
    Q_INVOKABLE virtual void setCenter(double latitude, double longitude) = 0;
    Q_INVOKABLE virtual void setZoom(double zoom) = 0;
    Q_INVOKABLE virtual void setPitch(double pitch) = 0;
    Q_INVOKABLE virtual void setBearing(double bearing) = 0;
    Q_INVOKABLE virtual void flyTo(double latitude, double longitude, 
                                    double zoom = -1, int durationMs = 1000) = 0;

    // 图层管理
    Q_INVOKABLE virtual void addGeoJSONLayer(const QString& layerId, 
                                              const QJsonObject& geoJson,
                                              const QVariantMap& style = {}) = 0;
    Q_INVOKABLE virtual void removeLayer(const QString& layerId) = 0;
    Q_INVOKABLE virtual void setLayerVisibility(const QString& layerId, bool visible) = 0;
    Q_INVOKABLE virtual void updateLayerData(const QString& layerId, 
                                              const QJsonObject& geoJson) = 0;

    // 样式管理
    Q_INVOKABLE virtual void setStyle(const QString& styleUrl) = 0;
    Q_INVOKABLE virtual QString currentStyle() const = 0;

    // 坐标转换
    Q_INVOKABLE virtual QPointF coordinateToScreen(double latitude, double longitude) const = 0;
    Q_INVOKABLE virtual QGeoCoordinate screenToCoordinate(double x, double y) const = 0;

signals:
    void readyChanged(bool ready);
    void mapClicked(double latitude, double longitude);
    void mapDoubleClicked(double latitude, double longitude);
    void centerChanged(double latitude, double longitude);
    void zoomChanged(double zoom);
    void pitchChanged(double pitch);
    void bearingChanged(double bearing);
    void layerAdded(const QString& layerId);
    void layerRemoved(const QString& layerId);
    void styleChanged(const QString& styleUrl);
};

} // namespace YEFS

#endif // YEFS_IMAPENGINE_H
