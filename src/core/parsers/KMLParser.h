#ifndef YEFS_KMLPARSER_H
#define YEFS_KMLPARSER_H

#include "../IMapParser.h"
#include "../IMapSource.h"
#include <QXmlStreamReader>
#include <QJsonObject>
#include <QGeoCoordinate>

namespace YEFS {

/**
 * @brief KML 地标
 */
struct KMLPlacemark {
    QString name;
    QString description;
    QString styleUrl;
    
    // 几何数据
    enum GeometryType {
        Point,
        LineString,
        Polygon,
        MultiGeometry
    };
    
    GeometryType geometryType;
    QList<QGeoCoordinate> coordinates;
    QList<QList<QGeoCoordinate>> innerBoundaries; // For Polygon
    
    QJsonObject toGeoJSONFeature() const;
};

/**
 * @brief KML 数据源
 */
class KMLSource : public IVectorMapSource
{
    Q_OBJECT

public:
    explicit KMLSource(const QString& id, const QString& name, QObject* parent = nullptr);
    ~KMLSource() override = default;

    // IMapSource 接口实现
    QString id() const override { return m_id; }
    QString name() const override { return m_name; }
    MapSourceType type() const override { return MapSourceType::Vector; }
    bool isLoaded() const override { return m_loaded; }
    bool isValid() const override { return !m_placemarks.isEmpty(); }
    QGeoRectangle bounds() const override { return m_bounds; }

    // 数据访问
    QJsonObject toGeoJSON() const override;
    QVariantMap toMapLibreLayer() const override;

    // IVectorMapSource 接口实现
    QJsonObject features() const override;
    int featureCount() const override { return m_placemarks.count(); }
    QVariantMap defaultStyle() const override;

    // KML 特有数据
    void addPlacemark(const KMLPlacemark& placemark);
    const QList<KMLPlacemark>& placemarks() const { return m_placemarks; }

private:
    void updateBounds();

    QString m_id;
    QString m_name;
    QList<KMLPlacemark> m_placemarks;
    QGeoRectangle m_bounds;
    bool m_loaded = false;
};

/**
 * @brief KML 格式解析器
 * 
 * 支持 KML 2.2/2.3 标准格式
 */
class KMLParser : public IMapParser
{
    Q_OBJECT

public:
    explicit KMLParser(QObject* parent = nullptr);
    ~KMLParser() override = default;

    // IMapParser 接口实现
    QString name() const override { return QStringLiteral("KML"); }
    QString description() const override { 
        return QStringLiteral("KML 格式解析器，支持 Google Earth KML/KMZ");
    }
    QStringList supportedExtensions() const override { 
        return {QStringLiteral("kml"), QStringLiteral("kmz")};
    }
    QStringList mimeTypes() const override {
        return {QStringLiteral("application/vnd.google-earth.kml+xml"),
                QStringLiteral("application/vnd.google-earth.kmz")};
    }

    bool canParse(const QString& filePath) const override;
    bool canParse(QIODevice* device) const override;

    IMapSource* parse(const QString& filePath) override;
    IMapSource* parse(QIODevice* device, const QString& sourceName) override;

private:
    KMLSource* parseKML(QXmlStreamReader& xml, const QString& sourceName);
    KMLPlacemark parsePlacemark(QXmlStreamReader& xml);
    QList<QGeoCoordinate> parseCoordinates(const QString& coordString);
    KMLPlacemark parsePoint(QXmlStreamReader& xml);
    KMLPlacemark parseLineString(QXmlStreamReader& xml);
    KMLPlacemark parsePolygon(QXmlStreamReader& xml);
};

} // namespace YEFS

#endif // YEFS_KMLPARSER_H
