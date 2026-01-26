#include "GeoJSONParser.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QDebug>
#include <QUuid>

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
    // 简单实现：遍历所有坐标计算边界
    // TODO: 实现完整的边界计算
    m_bounds = QGeoRectangle(QGeoCoordinate(90, -180), QGeoCoordinate(-90, 180));
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
