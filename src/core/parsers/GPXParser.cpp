#include "GPXParser.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QDebug>
#include <QUuid>
#include <QtMath>

namespace YEFS {

// ============================================================================
// GPX 数据结构实现
// ============================================================================

QJsonObject GPXTrackPoint::toJson() const
{
    QJsonObject obj;
    obj["lat"] = coordinate.latitude();
    obj["lon"] = coordinate.longitude();
    if (elevation != 0.0) {
        obj["ele"] = elevation;
    }
    if (time.isValid()) {
        obj["time"] = time.toString(Qt::ISODate);
    }
    if (speed > 0.0) {
        obj["speed"] = speed;
    }
    if (heartRate > 0.0) {
        obj["hr"] = heartRate;
    }
    return obj;
}

QJsonObject GPXTrackSegment::toJson() const
{
    QJsonArray pointsArray;
    for (const auto& point : points) {
        pointsArray.append(point.toJson());
    }
    QJsonObject obj;
    obj["points"] = pointsArray;
    return obj;
}

QJsonObject GPXTrack::toJson() const
{
    QJsonObject obj;
    obj["name"] = name;
    obj["description"] = description;
    
    QJsonArray segmentsArray;
    for (const auto& segment : segments) {
        segmentsArray.append(segment.toJson());
    }
    obj["segments"] = segmentsArray;
    return obj;
}

QJsonObject GPXWaypoint::toJson() const
{
    QJsonObject obj;
    obj["lat"] = coordinate.latitude();
    obj["lon"] = coordinate.longitude();
    obj["name"] = name;
    obj["description"] = description;
    obj["symbol"] = symbol;
    if (elevation != 0.0) {
        obj["ele"] = elevation;
    }
    if (time.isValid()) {
        obj["time"] = time.toString(Qt::ISODate);
    }
    return obj;
}

// ============================================================================
// GPXSource 实现
// ============================================================================

GPXSource::GPXSource(const QString& id, const QString& name, QObject* parent)
    : IVectorMapSource(parent)
    , m_id(id)
    , m_name(name)
    , m_loaded(true)
{
}

void GPXSource::addTrack(const GPXTrack& track)
{
    m_tracks.append(track);
    updateBounds();
}

void GPXSource::addWaypoint(const GPXWaypoint& waypoint)
{
    m_waypoints.append(waypoint);
    updateBounds();
}

void GPXSource::updateBounds()
{
    if (m_tracks.isEmpty() && m_waypoints.isEmpty()) {
        return;
    }

    double minLat = 90.0, maxLat = -90.0;
    double minLon = 180.0, maxLon = -180.0;

    // 从轨迹中计算边界
    for (const auto& track : m_tracks) {
        for (const auto& segment : track.segments) {
            for (const auto& point : segment.points) {
                minLat = qMin(minLat, point.coordinate.latitude());
                maxLat = qMax(maxLat, point.coordinate.latitude());
                minLon = qMin(minLon, point.coordinate.longitude());
                maxLon = qMax(maxLon, point.coordinate.longitude());
            }
        }
    }

    // 从航点中计算边界
    for (const auto& waypoint : m_waypoints) {
        minLat = qMin(minLat, waypoint.coordinate.latitude());
        maxLat = qMax(maxLat, waypoint.coordinate.latitude());
        minLon = qMin(minLon, waypoint.coordinate.longitude());
        maxLon = qMax(maxLon, waypoint.coordinate.longitude());
    }

    m_bounds = QGeoRectangle(QGeoCoordinate(maxLat, minLon), 
                             QGeoCoordinate(minLat, maxLon));
}

QJsonObject GPXSource::toGeoJSON() const
{
    QJsonObject geoJson;
    geoJson["type"] = "FeatureCollection";

    QJsonArray features;

    // 转换轨迹为 LineString
    for (const auto& track : m_tracks) {
        for (const auto& segment : track.segments) {
            QJsonObject feature;
            feature["type"] = "Feature";

            QJsonObject geometry;
            geometry["type"] = "LineString";

            QJsonArray coordinates;
            for (const auto& point : segment.points) {
                QJsonArray coord;
                coord.append(point.coordinate.longitude());
                coord.append(point.coordinate.latitude());
                if (point.elevation != 0.0) {
                    coord.append(point.elevation);
                }
                coordinates.append(coord);
            }
            geometry["coordinates"] = coordinates;

            feature["geometry"] = geometry;

            QJsonObject properties;
            properties["name"] = track.name;
            properties["description"] = track.description;
            feature["properties"] = properties;

            features.append(feature);
        }
    }

    // 转换航点为 Point
    for (const auto& waypoint : m_waypoints) {
        QJsonObject feature;
        feature["type"] = "Feature";

        QJsonObject geometry;
        geometry["type"] = "Point";

        QJsonArray coordinate;
        coordinate.append(waypoint.coordinate.longitude());
        coordinate.append(waypoint.coordinate.latitude());
        if (waypoint.elevation != 0.0) {
            coordinate.append(waypoint.elevation);
        }
        geometry["coordinates"] = coordinate;

        feature["geometry"] = geometry;

        QJsonObject properties;
        properties["name"] = waypoint.name;
        properties["description"] = waypoint.description;
        properties["symbol"] = waypoint.symbol;
        feature["properties"] = properties;

        features.append(feature);
    }

    geoJson["features"] = features;
    return geoJson;
}

QVariantMap GPXSource::toMapLibreLayer() const
{
    QVariantMap layer;
    layer["id"] = m_id;
    layer["type"] = "geojson";
    layer["source"] = QVariant::fromValue(toGeoJSON().toVariantMap());
    return layer;
}

QJsonObject GPXSource::features() const
{
    return toGeoJSON();
}

int GPXSource::featureCount() const
{
    int count = m_waypoints.count();
    for (const auto& track : m_tracks) {
        count += track.segments.count();
    }
    return count;
}

QVariantMap GPXSource::defaultStyle() const
{
    QVariantMap style;
    style["type"] = "line";
    style["paint"] = QVariantMap{
        {"line-color", "#ff3388"},
        {"line-width", 3}
    };
    return style;
}

// ============================================================================
// GPXParser 实现
// ============================================================================

GPXParser::GPXParser(QObject* parent)
    : IMapParser(parent)
{
}

bool GPXParser::canParse(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    return canParse(&file);
}

bool GPXParser::canParse(QIODevice* device) const
{
    if (!device || !device->isReadable()) {
        return false;
    }

    qint64 originalPos = device->pos();
    device->seek(0);

    QXmlStreamReader xml(device);
    bool isGPX = false;

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == QStringLiteral("gpx")) {
            isGPX = true;
            break;
        }
    }

    device->seek(originalPos);
    return isGPX;
}

IMapSource* GPXParser::parse(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[GPXParser] Cannot open file:" << filePath;
        emit parseError(QStringLiteral("无法打开文件"));
        return nullptr;
    }

    QFileInfo fileInfo(filePath);
    return parse(&file, fileInfo.fileName());
}

IMapSource* GPXParser::parse(QIODevice* device, const QString& sourceName)
{
    if (!device || !device->isReadable()) {
        qWarning() << "[GPXParser] Invalid device";
        emit parseError(QStringLiteral("无效的数据源"));
        return nullptr;
    }

    device->seek(0);
    QXmlStreamReader xml(device);

    return parseGPX(xml, sourceName);
}

GPXSource* GPXParser::parseGPX(QXmlStreamReader& xml, const QString& sourceName)
{
    QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QString name = sourceName.isEmpty() ? QStringLiteral("GPX") : sourceName;

    auto source = new GPXSource(id, name);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QStringLiteral("trk")) {
                source->addTrack(parseTrack(xml));
            }
            else if (xml.name() == QStringLiteral("wpt")) {
                source->addWaypoint(parseWaypoint(xml));
            }
        }
    }

    if (xml.hasError()) {
        qWarning() << "[GPXParser] XML parse error:" << xml.errorString();
        emit parseError(QStringLiteral("XML 解析错误: ") + xml.errorString());
        delete source;
        return nullptr;
    }

    qDebug() << "[GPXParser] Parsed GPX:" << name 
             << "tracks:" << source->tracks().count()
             << "waypoints:" << source->waypoints().count();

    return source;
}

GPXTrack GPXParser::parseTrack(QXmlStreamReader& xml)
{
    GPXTrack track;

    while (!(xml.isEndElement() && xml.name() == QStringLiteral("trk"))) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QStringLiteral("name")) {
                track.name = xml.readElementText();
            }
            else if (xml.name() == QStringLiteral("desc")) {
                track.description = xml.readElementText();
            }
            else if (xml.name() == QStringLiteral("trkseg")) {
                track.segments.append(parseTrackSegment(xml));
            }
        }
    }

    return track;
}

GPXTrackSegment GPXParser::parseTrackSegment(QXmlStreamReader& xml)
{
    GPXTrackSegment segment;

    while (!(xml.isEndElement() && xml.name() == QStringLiteral("trkseg"))) {
        xml.readNext();

        if (xml.isStartElement() && xml.name() == QStringLiteral("trkpt")) {
            segment.points.append(parseTrackPoint(xml));
        }
    }

    return segment;
}

GPXTrackPoint GPXParser::parseTrackPoint(QXmlStreamReader& xml)
{
    GPXTrackPoint point;
    point.coordinate = parseCoordinate(xml);

    while (!(xml.isEndElement() && xml.name() == QStringLiteral("trkpt"))) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QStringLiteral("ele")) {
                point.elevation = xml.readElementText().toDouble();
            }
            else if (xml.name() == QStringLiteral("time")) {
                point.time = QDateTime::fromString(xml.readElementText(), Qt::ISODate);
            }
            else if (xml.name() == QStringLiteral("speed")) {
                point.speed = xml.readElementText().toDouble();
            }
        }
    }

    return point;
}

GPXWaypoint GPXParser::parseWaypoint(QXmlStreamReader& xml)
{
    GPXWaypoint waypoint;
    waypoint.coordinate = parseCoordinate(xml);

    while (!(xml.isEndElement() && xml.name() == QStringLiteral("wpt"))) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QStringLiteral("name")) {
                waypoint.name = xml.readElementText();
            }
            else if (xml.name() == QStringLiteral("desc")) {
                waypoint.description = xml.readElementText();
            }
            else if (xml.name() == QStringLiteral("sym")) {
                waypoint.symbol = xml.readElementText();
            }
            else if (xml.name() == QStringLiteral("ele")) {
                waypoint.elevation = xml.readElementText().toDouble();
            }
            else if (xml.name() == QStringLiteral("time")) {
                waypoint.time = QDateTime::fromString(xml.readElementText(), Qt::ISODate);
            }
        }
    }

    return waypoint;
}

QGeoCoordinate GPXParser::parseCoordinate(QXmlStreamReader& xml)
{
    QXmlStreamAttributes attrs = xml.attributes();
    double lat = attrs.value("lat").toDouble();
    double lon = attrs.value("lon").toDouble();
    return QGeoCoordinate(lat, lon);
}

} // namespace YEFS
