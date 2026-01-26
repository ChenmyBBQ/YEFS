#ifndef YEFS_GEOJSONPARSER_H
#define YEFS_GEOJSONPARSER_H

#include "../IMapParser.h"
#include "../IMapSource.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QGeoRectangle>
#include <QFileInfo>

namespace YEFS {

/**
 * @brief GeoJSON 数据源
 */
class GeoJSONSource : public IVectorMapSource
{
    Q_OBJECT

public:
    explicit GeoJSONSource(const QString& id, const QString& name, 
                          const QJsonObject& geoJson, QObject* parent = nullptr);
    ~GeoJSONSource() override = default;

    // IMapSource 接口实现
    QString id() const override { return m_id; }
    QString name() const override { return m_name; }
    MapSourceType type() const override { return MapSourceType::Vector; }
    bool isLoaded() const override { return m_loaded; }
    bool isValid() const override { return !m_geoJson.isEmpty(); }
    QGeoRectangle bounds() const override { return m_bounds; }

    // 数据访问
    QJsonObject toGeoJSON() const override { return m_geoJson; }
    QVariantMap toMapLibreLayer() const override;

    // IVectorMapSource 接口实现
    QJsonObject features() const override;
    int featureCount() const override;
    QVariantMap defaultStyle() const override;

private:
    void calculateBounds();

    QString m_id;
    QString m_name;
    QJsonObject m_geoJson;
    QGeoRectangle m_bounds;
    bool m_loaded = false;
};

/**
 * @brief GeoJSON 格式解析器
 */
class GeoJSONParser : public IMapParser
{
    Q_OBJECT

public:
    explicit GeoJSONParser(QObject* parent = nullptr);
    ~GeoJSONParser() override = default;

    // IMapParser 接口实现
    QString name() const override { return QStringLiteral("GeoJSON"); }
    QString description() const override { 
        return QStringLiteral("GeoJSON 格式解析器，支持标准 GeoJSON 和 TopoJSON");
    }
    QStringList supportedExtensions() const override { 
        return {QStringLiteral("geojson"), QStringLiteral("json"), QStringLiteral("topojson")};
    }
    QStringList mimeTypes() const override {
        return {QStringLiteral("application/geo+json"), 
                QStringLiteral("application/json")};
    }

    bool canParse(const QString& filePath) const override;
    bool canParse(QIODevice* device) const override;

    IMapSource* parse(const QString& filePath) override;
    IMapSource* parse(QIODevice* device, const QString& sourceName) override;

private:
    IMapSource* parseGeoJSON(const QJsonObject& json, const QString& sourceName);
    bool isValidGeoJSON(const QJsonObject& json) const;
};

} // namespace YEFS

#endif // YEFS_GEOJSONPARSER_H
