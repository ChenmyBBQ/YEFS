#include "KMLParser.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QDebug>
#include <QUuid>
#include <QRegularExpression>

namespace YEFS {

// ============================================================================
// KMLPlacemark 实现
// ============================================================================

QJsonObject KMLPlacemark::toGeoJSONFeature() const
{
    QJsonObject feature;
    feature["type"] = "Feature";

    // 几何数据
    QJsonObject geometry;
    switch (geometryType) {
    case Point:
        geometry["type"] = "Point";
        if (!coordinates.isEmpty()) {
            QJsonArray coord;
            coord.append(coordinates[0].longitude());
            coord.append(coordinates[0].latitude());
            coord.append(coordinates[0].altitude());
            geometry["coordinates"] = coord;
        }
        break;
        
    case LineString:
        geometry["type"] = "LineString";
        {
            QJsonArray coords;
            for (const auto& coord : coordinates) {
                QJsonArray point;
                point.append(coord.longitude());
                point.append(coord.latitude());
                point.append(coord.altitude());
                coords.append(point);
            }
            geometry["coordinates"] = coords;
        }
        break;
        
    case Polygon:
        geometry["type"] = "Polygon";
        {
            QJsonArray rings;
            // 外环
            QJsonArray outerRing;
            for (const auto& coord : coordinates) {
                QJsonArray point;
                point.append(coord.longitude());
                point.append(coord.latitude());
                point.append(coord.altitude());
                outerRing.append(point);
            }
            rings.append(outerRing);
            
            // 内环
            for (const auto& inner : innerBoundaries) {
                QJsonArray innerRing;
                for (const auto& coord : inner) {
                    QJsonArray point;
                    point.append(coord.longitude());
                    point.append(coord.latitude());
                    point.append(coord.altitude());
                    innerRing.append(point);
                }
                rings.append(innerRing);
            }
            
            geometry["coordinates"] = rings;
        }
        break;
        
    default:
        break;
    }
    
    feature["geometry"] = geometry;

    // 属性
    QJsonObject properties;
    properties["name"] = name;
    properties["description"] = description;
    properties["styleUrl"] = styleUrl;
    feature["properties"] = properties;

    return feature;
}

// ============================================================================
// KMLSource 实现
// ============================================================================

KMLSource::KMLSource(const QString& id, const QString& name, QObject* parent)
    : IVectorMapSource(parent)
    , m_id(id)
    , m_name(name)
    , m_loaded(true)
{
}

void KMLSource::addPlacemark(const KMLPlacemark& placemark)
{
    m_placemarks.append(placemark);
    updateBounds();
}

void KMLSource::updateBounds()
{
    if (m_placemarks.isEmpty()) {
        return;
    }

    double minLat = 90.0, maxLat = -90.0;
    double minLon = 180.0, maxLon = -180.0;

    for (const auto& placemark : m_placemarks) {
        for (const auto& coord : placemark.coordinates) {
            minLat = qMin(minLat, coord.latitude());
            maxLat = qMax(maxLat, coord.latitude());
            minLon = qMin(minLon, coord.longitude());
            maxLon = qMax(maxLon, coord.longitude());
        }
    }

    m_bounds = QGeoRectangle(QGeoCoordinate(maxLat, minLon), 
                             QGeoCoordinate(minLat, maxLon));
}

QJsonObject KMLSource::toGeoJSON() const
{
    QJsonObject geoJson;
    geoJson["type"] = "FeatureCollection";

    QJsonArray features;
    for (const auto& placemark : m_placemarks) {
        features.append(placemark.toGeoJSONFeature());
    }

    geoJson["features"] = features;
    return geoJson;
}

QVariantMap KMLSource::toMapLibreLayer() const
{
    QVariantMap layer;
    layer["id"] = m_id;
    layer["type"] = "geojson";
    layer["source"] = QVariant::fromValue(toGeoJSON().toVariantMap());
    return layer;
}

QJsonObject KMLSource::features() const
{
    return toGeoJSON();
}

QVariantMap KMLSource::defaultStyle() const
{
    QVariantMap style;
    style["type"] = "line";
    style["paint"] = QVariantMap{
        {"line-color", "#ffaa00"},
        {"line-width", 2}
    };
    return style;
}

// ============================================================================
// KMLParser 实现
// ============================================================================

KMLParser::KMLParser(QObject* parent)
    : IMapParser(parent)
{
}

bool KMLParser::canParse(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    return canParse(&file);
}

bool KMLParser::canParse(QIODevice* device) const
{
    if (!device || !device->isReadable()) {
        return false;
    }

    qint64 originalPos = device->pos();
    device->seek(0);

    QXmlStreamReader xml(device);
    bool isKML = false;

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();
        if (xml.isStartElement() && xml.name() == QStringLiteral("kml")) {
            isKML = true;
            break;
        }
    }

    device->seek(originalPos);
    return isKML;
}

IMapSource* KMLParser::parse(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[KMLParser] Cannot open file:" << filePath;
        emit parseError(QStringLiteral("无法打开文件"));
        return nullptr;
    }

    QFileInfo fileInfo(filePath);
    return parse(&file, fileInfo.fileName());
}

IMapSource* KMLParser::parse(QIODevice* device, const QString& sourceName)
{
    if (!device || !device->isReadable()) {
        qWarning() << "[KMLParser] Invalid device";
        emit parseError(QStringLiteral("无效的数据源"));
        return nullptr;
    }

    device->seek(0);
    QXmlStreamReader xml(device);

    return parseKML(xml, sourceName);
}

KMLSource* KMLParser::parseKML(QXmlStreamReader& xml, const QString& sourceName)
{
    QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QString name = sourceName.isEmpty() ? QStringLiteral("KML") : sourceName;

    auto source = new KMLSource(id, name);

    while (!xml.atEnd() && !xml.hasError()) {
        xml.readNext();

        if (xml.isStartElement() && xml.name() == QStringLiteral("Placemark")) {
            source->addPlacemark(parsePlacemark(xml));
        }
    }

    if (xml.hasError()) {
        qWarning() << "[KMLParser] XML parse error:" << xml.errorString();
        emit parseError(QStringLiteral("XML 解析错误: ") + xml.errorString());
        delete source;
        return nullptr;
    }

    qDebug() << "[KMLParser] Parsed KML:" << name 
             << "placemarks:" << source->placemarks().count();

    return source;
}

KMLPlacemark KMLParser::parsePlacemark(QXmlStreamReader& xml)
{
    KMLPlacemark placemark;
    QString name, description, styleUrl;

    while (!(xml.isEndElement() && xml.name() == QStringLiteral("Placemark"))) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QStringLiteral("name")) {
                name = xml.readElementText();
            }
            else if (xml.name() == QStringLiteral("description")) {
                description = xml.readElementText();
            }
            else if (xml.name() == QStringLiteral("styleUrl")) {
                styleUrl = xml.readElementText();
            }
            else if (xml.name() == QStringLiteral("Point")) {
                placemark = parsePoint(xml);
                placemark.name = name;
                placemark.description = description;
                placemark.styleUrl = styleUrl;
            }
            else if (xml.name() == QStringLiteral("LineString")) {
                placemark = parseLineString(xml);
                placemark.name = name;
                placemark.description = description;
                placemark.styleUrl = styleUrl;
            }
            else if (xml.name() == QStringLiteral("Polygon")) {
                placemark = parsePolygon(xml);
                placemark.name = name;
                placemark.description = description;
                placemark.styleUrl = styleUrl;
            }
        }
    }

    return placemark;
}

KMLPlacemark KMLParser::parsePoint(QXmlStreamReader& xml)
{
    KMLPlacemark placemark;
    placemark.geometryType = KMLPlacemark::Point;

    while (!(xml.isEndElement() && xml.name() == QStringLiteral("Point"))) {
        xml.readNext();

        if (xml.isStartElement() && xml.name() == QStringLiteral("coordinates")) {
            QString coordString = xml.readElementText();
            placemark.coordinates = parseCoordinates(coordString);
        }
    }

    return placemark;
}

KMLPlacemark KMLParser::parseLineString(QXmlStreamReader& xml)
{
    KMLPlacemark placemark;
    placemark.geometryType = KMLPlacemark::LineString;

    while (!(xml.isEndElement() && xml.name() == QStringLiteral("LineString"))) {
        xml.readNext();

        if (xml.isStartElement() && xml.name() == QStringLiteral("coordinates")) {
            QString coordString = xml.readElementText();
            placemark.coordinates = parseCoordinates(coordString);
        }
    }

    return placemark;
}

KMLPlacemark KMLParser::parsePolygon(QXmlStreamReader& xml)
{
    KMLPlacemark placemark;
    placemark.geometryType = KMLPlacemark::Polygon;

    while (!(xml.isEndElement() && xml.name() == QStringLiteral("Polygon"))) {
        xml.readNext();

        if (xml.isStartElement()) {
            if (xml.name() == QStringLiteral("outerBoundaryIs")) {
                while (!(xml.isEndElement() && xml.name() == QStringLiteral("outerBoundaryIs"))) {
                    xml.readNext();
                    if (xml.isStartElement() && xml.name() == QStringLiteral("coordinates")) {
                        QString coordString = xml.readElementText();
                        placemark.coordinates = parseCoordinates(coordString);
                    }
                }
            }
            else if (xml.name() == QStringLiteral("innerBoundaryIs")) {
                while (!(xml.isEndElement() && xml.name() == QStringLiteral("innerBoundaryIs"))) {
                    xml.readNext();
                    if (xml.isStartElement() && xml.name() == QStringLiteral("coordinates")) {
                        QString coordString = xml.readElementText();
                        placemark.innerBoundaries.append(parseCoordinates(coordString));
                    }
                }
            }
        }
    }

    return placemark;
}

QList<QGeoCoordinate> KMLParser::parseCoordinates(const QString& coordString)
{
    QList<QGeoCoordinate> coordinates;
    
    QString trimmed = coordString.trimmed();
    QStringList points = trimmed.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

    for (const QString& point : points) {
        QStringList parts = point.split(',');
        if (parts.size() >= 2) {
            double lon = parts[0].toDouble();
            double lat = parts[1].toDouble();
            double alt = parts.size() >= 3 ? parts[2].toDouble() : 0.0;
            coordinates.append(QGeoCoordinate(lat, lon, alt));
        }
    }

    return coordinates;
}

} // namespace YEFS
