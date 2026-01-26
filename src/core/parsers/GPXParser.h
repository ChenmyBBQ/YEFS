#ifndef YEFS_GPXPARSER_H
#define YEFS_GPXPARSER_H

#include "../IMapParser.h"
#include "../IMapSource.h"
#include <QXmlStreamReader>
#include <QJsonObject>
#include <QGeoCoordinate>
#include <QDateTime>

namespace YEFS {

/**
 * @brief GPX 轨迹点
 */
struct GPXTrackPoint {
    QGeoCoordinate coordinate;
    QDateTime time;
    double elevation = 0.0;
    double speed = 0.0;
    double heartRate = 0.0;
    
    QJsonObject toJson() const;
};

/**
 * @brief GPX 轨迹段
 */
struct GPXTrackSegment {
    QList<GPXTrackPoint> points;
    
    QJsonObject toJson() const;
};

/**
 * @brief GPX 轨迹
 */
struct GPXTrack {
    QString name;
    QString description;
    QList<GPXTrackSegment> segments;
    
    QJsonObject toJson() const;
};

/**
 * @brief GPX 航点
 */
struct GPXWaypoint {
    QGeoCoordinate coordinate;
    QString name;
    QString description;
    QString symbol;
    double elevation = 0.0;
    QDateTime time;
    
    QJsonObject toJson() const;
};

/**
 * @brief GPX 数据源
 */
class GPXSource : public IVectorMapSource
{
    Q_OBJECT

public:
    explicit GPXSource(const QString& id, const QString& name, QObject* parent = nullptr);
    ~GPXSource() override = default;

    // IMapSource 接口实现
    QString id() const override { return m_id; }
    QString name() const override { return m_name; }
    MapSourceType type() const override { return MapSourceType::Vector; }
    bool isLoaded() const override { return m_loaded; }
    bool isValid() const override { return !m_tracks.isEmpty() || !m_waypoints.isEmpty(); }
    QGeoRectangle bounds() const override { return m_bounds; }

    // 数据访问
    QJsonObject toGeoJSON() const override;
    QVariantMap toMapLibreLayer() const override;

    // IVectorMapSource 接口实现
    QJsonObject features() const override;
    int featureCount() const override;
    QVariantMap defaultStyle() const override;

    // GPX 特有数据
    void addTrack(const GPXTrack& track);
    void addWaypoint(const GPXWaypoint& waypoint);
    const QList<GPXTrack>& tracks() const { return m_tracks; }
    const QList<GPXWaypoint>& waypoints() const { return m_waypoints; }

private:
    void updateBounds();

    QString m_id;
    QString m_name;
    QList<GPXTrack> m_tracks;
    QList<GPXWaypoint> m_waypoints;
    QGeoRectangle m_bounds;
    bool m_loaded = false;
};

/**
 * @brief GPX 格式解析器
 * 
 * 支持 GPX 1.0 和 GPX 1.1 标准格式
 */
class GPXParser : public IMapParser
{
    Q_OBJECT

public:
    explicit GPXParser(QObject* parent = nullptr);
    ~GPXParser() override = default;

    // IMapParser 接口实现
    QString name() const override { return QStringLiteral("GPX"); }
    QString description() const override { 
        return QStringLiteral("GPX 轨迹格式解析器，支持 GPX 1.0/1.1");
    }
    QStringList supportedExtensions() const override { 
        return {QStringLiteral("gpx")};
    }
    QStringList mimeTypes() const override {
        return {QStringLiteral("application/gpx+xml")};
    }

    bool canParse(const QString& filePath) const override;
    bool canParse(QIODevice* device) const override;

    IMapSource* parse(const QString& filePath) override;
    IMapSource* parse(QIODevice* device, const QString& sourceName) override;

private:
    GPXSource* parseGPX(QXmlStreamReader& xml, const QString& sourceName);
    GPXTrack parseTrack(QXmlStreamReader& xml);
    GPXTrackSegment parseTrackSegment(QXmlStreamReader& xml);
    GPXTrackPoint parseTrackPoint(QXmlStreamReader& xml);
    GPXWaypoint parseWaypoint(QXmlStreamReader& xml);
    QGeoCoordinate parseCoordinate(QXmlStreamReader& xml);
};

} // namespace YEFS

#endif // YEFS_GPXPARSER_H
