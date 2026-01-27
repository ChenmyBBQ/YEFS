#ifndef YEFS_IMAPSOURCE_H
#define YEFS_IMAPSOURCE_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QGeoCoordinate>
#include <QGeoRectangle>
#include <QJsonObject>
#include <QVariantMap>
#include <QImage>

namespace YEFS {

/**
 * @brief 地图数据源类型
 */
enum class MapSourceType {
    Vector,         // 矢量数据（GeoJSON、KML、GPX等）
    Raster,         // 栅格瓦片（MBTiles、本地瓦片）
    Online,         // 在线地图服务（WMS、WMTS、XYZ）
    Terrain,        // 地形数据（DEM）
    Custom          // 自定义数据源
};

/**
 * @brief 地图数据源接口
 * 
 * 定义了地图数据源的基本属性和操作
 */
class IMapSource : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(MapSourceType type READ type CONSTANT)
    Q_PROPERTY(bool isOnline READ isOnline CONSTANT)
    Q_PROPERTY(bool isLoaded READ isLoaded NOTIFY loadedChanged)

public:
    explicit IMapSource(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IMapSource() = default;

    // 基本信息
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString description() const { return QString(); }
    virtual MapSourceType type() const = 0;
    virtual bool isOnline() const { return false; }

    // 状态
    virtual bool isLoaded() const = 0;
    virtual bool isValid() const = 0;

    // 地理范围
    virtual QGeoRectangle bounds() const = 0;
    virtual int minZoom() const { return 0; }
    virtual int maxZoom() const { return 18; }

    // 数据访问
    virtual QJsonObject toGeoJSON() const { return QJsonObject(); }
    virtual QVariantMap toMapLibreLayer() const { return QVariantMap(); }

signals:
    void loadedChanged(bool loaded);
    void dataChanged();
    void error(const QString& message);
};

/**
 * @brief 矢量数据源接口
 */
class IVectorMapSource : public IMapSource
{
    Q_OBJECT

public:
    explicit IVectorMapSource(QObject* parent = nullptr) : IMapSource(parent) {}
    virtual ~IVectorMapSource() = default;

    MapSourceType type() const override { return MapSourceType::Vector; }

    // 矢量数据特有功能
    virtual QJsonObject features() const = 0;
    virtual int featureCount() const = 0;
    
    // 样式
    virtual QVariantMap defaultStyle() const { return QVariantMap(); }
};

/**
 * @brief 栅格瓦片数据源接口
 */
class IRasterMapSource : public IMapSource
{
    Q_OBJECT

public:
    explicit IRasterMapSource(QObject* parent = nullptr) : IMapSource(parent) {}
    virtual ~IRasterMapSource() = default;

    MapSourceType type() const override { return MapSourceType::Raster; }

    // 瓦片访问
    virtual QImage tile(int z, int x, int y) const = 0;
    virtual QString tileUrl(int z, int x, int y) const = 0;
    
    // 瓦片格式
    virtual QString format() const { return QStringLiteral("png"); }
    virtual int tileSize() const { return 256; }
};

/**
 * @brief 在线地图数据源接口
 */
class IOnlineMapSource : public IRasterMapSource
{
    Q_OBJECT
    Q_PROPERTY(QString urlTemplate READ urlTemplate CONSTANT)
    Q_PROPERTY(bool requiresApiKey READ requiresApiKey CONSTANT)

public:
    explicit IOnlineMapSource(QObject* parent = nullptr) : IRasterMapSource(parent) {}
    virtual ~IOnlineMapSource() = default;

    bool isOnline() const override { return true; }

    // URL 模板
    virtual QString urlTemplate() const = 0;
    virtual bool requiresApiKey() const { return false; }
    virtual void setApiKey(const QString& key) { Q_UNUSED(key); }
    
    // 服务信息
    virtual QString attribution() const { return QString(); }
    virtual QUrl termsOfServiceUrl() const { return QUrl(); }
};

} // namespace YEFS

Q_DECLARE_METATYPE(YEFS::MapSourceType)

#endif // YEFS_IMAPSOURCE_H
