#include "GeoJSONParser.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>
#include <QUuid>
#include <functional>

namespace YEFS {

// ============================================================================
// GeoJSONSource 实现
// ============================================================================

GeoJSONSource::GeoJSONSource(const QString& id, const QString& name, 
                            const QJsonObject& geoJson, QObject* parent)
    : IVectorMapSource(parent)
    , m_id(id)
    , m_name(name)
    , m_geoJson(geoJson)
    , m_loaded(true)
{
    calculateBounds();
}

void GeoJSONSource::calculateBounds()
{
    // 遍历 GeoJSON 计算实际边界
    if (m_geoJson.isEmpty()) {
        m_bounds = QGeoRectangle();
        return;
    }

    double minLat = 90.0, maxLat = -90.0;
    double minLon = 180.0, maxLon = -180.0;
    bool hasCoords = false;

    // 递归函数处理坐标
    std::function<void(const QJsonValue&)> processCoordinates;
    processCoordinates = [&](const QJsonValue& coords) {
        if (coords.isArray()) {
            QJsonArray arr = coords.toArray();
            if (arr.isEmpty()) return;

            // 检查是否是坐标点 [lon, lat] 或 [lon, lat, alt]
            if (arr[0].isDouble()) {
                if (arr.size() >= 2) {
                    double lon = arr[0].toDouble();
                    double lat = arr[1].toDouble();
                    minLat = qMin(minLat, lat);
                    maxLat = qMax(maxLat, lat);
                    minLon = qMin(minLon, lon);
                    maxLon = qMax(maxLon, lon);
                    hasCoords = true;
                }
            } else {
                // 递归处理嵌套数组
                for (const auto& item : arr) {
                    processCoordinates(item);
                }
            }
        }
    };

    // 处理 Feature 或 FeatureCollection
    QString type = m_geoJson["type"].toString();
    if (type == "FeatureCollection") {
        QJsonArray features = m_geoJson["features"].toArray();
        for (const auto& feature : features) {
            QJsonObject featureObj = feature.toObject();
            QJsonObject geometry = featureObj["geometry"].toObject();
            processCoordinates(geometry["coordinates"]);
        }
    } else if (type == "Feature") {
        QJsonObject geometry = m_geoJson["geometry"].toObject();
        processCoordinates(geometry["coordinates"]);
    } else {
        // 直接是 Geometry
        processCoordinates(m_geoJson["coordinates"]);
    }

    if (hasCoords) {
        m_bounds = QGeoRectangle(QGeoCoordinate(maxLat, minLon), 
                                QGeoCoordinate(minLat, maxLon));
    } else {
        m_bounds = QGeoRectangle();
    }
}

QVariantMap GeoJSONSource::toMapLibreLayer() const
{
    QVariantMap layer;
    layer["id"] = m_id;
    layer["type"] = "geojson";
    layer["source"] = QVariant::fromValue(m_geoJson.toVariantMap());
    return layer;
}

QJsonObject GeoJSONSource::features() const
{
    return m_geoJson;
}

int GeoJSONSource::featureCount() const
{
    if (m_geoJson.contains("features")) {
        QJsonArray features = m_geoJson["features"].toArray();
        return features.count();
    }
    return 1; // 单个 Feature
}

QVariantMap GeoJSONSource::defaultStyle() const
{
    QVariantMap style;
    style["type"] = "line";
    style["paint"] = QVariantMap{
        {"line-color", "#3388ff"},
        {"line-width", 2}
    };
    return style;
}

// ============================================================================
// GeoJSONParser 实现
// ============================================================================

GeoJSONParser::GeoJSONParser(QObject* parent)
    : IMapParser(parent)
{
}

bool GeoJSONParser::canParse(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    return canParse(&file);
}

bool GeoJSONParser::canParse(QIODevice* device) const
{
    if (!device || !device->isReadable()) {
        return false;
    }

    qint64 originalPos = device->pos();
    device->seek(0);

    QByteArray data = device->peek(1024); // 只读取前 1KB
    device->seek(originalPos);

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        return false;
    }

    if (!doc.isObject()) {
        return false;
    }

    return isValidGeoJSON(doc.object());
}

IMapSource* GeoJSONParser::parse(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[GeoJSONParser] Cannot open file:" << filePath;
        emit parseError(QStringLiteral("无法打开文件"));
        return nullptr;
    }

    QFileInfo fileInfo(filePath);
    return parse(&file, fileInfo.fileName());
}

IMapSource* GeoJSONParser::parse(QIODevice* device, const QString& sourceName)
{
    if (!device || !device->isReadable()) {
        qWarning() << "[GeoJSONParser] Invalid device";
        emit parseError(QStringLiteral("无效的数据源"));
        return nullptr;
    }

    device->seek(0);
    QByteArray data = device->readAll();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError) {
        qWarning() << "[GeoJSONParser] JSON parse error:" << error.errorString();
        emit parseError(QStringLiteral("JSON 解析错误: ") + error.errorString());
        return nullptr;
    }

    if (!doc.isObject()) {
        qWarning() << "[GeoJSONParser] Invalid GeoJSON: not an object";
        emit parseError(QStringLiteral("无效的 GeoJSON 格式"));
        return nullptr;
    }

    return parseGeoJSON(doc.object(), sourceName);
}

IMapSource* GeoJSONParser::parseGeoJSON(const QJsonObject& json, const QString& sourceName)
{
    if (!isValidGeoJSON(json)) {
        qWarning() << "[GeoJSONParser] Invalid GeoJSON structure";
        emit parseError(QStringLiteral("无效的 GeoJSON 结构"));
        return nullptr;
    }

    QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QString name = sourceName.isEmpty() ? QStringLiteral("GeoJSON") : sourceName;

    auto source = new GeoJSONSource(id, name, json);
    qDebug() << "[GeoJSONParser] Parsed GeoJSON:" << name 
             << "features:" << source->featureCount();

    return source;
}

bool GeoJSONParser::isValidGeoJSON(const QJsonObject& json) const
{
    // 检查是否有 type 字段
    if (!json.contains("type")) {
        return false;
    }

    QString type = json["type"].toString();
    
    // 支持的 GeoJSON 类型
    QStringList validTypes = {
        "FeatureCollection", "Feature",
        "Point", "LineString", "Polygon",
        "MultiPoint", "MultiLineString", "MultiPolygon",
        "GeometryCollection"
    };

    return validTypes.contains(type);
}

} // namespace YEFS
