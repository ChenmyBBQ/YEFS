#include "AirspaceDisplayMapper.h"
#include "ShapeGenerator.h"
#include "AirspaceGeometryConverter.h"

#include "AirspacePayloadConverter.h"

QVariantMap AirspaceDisplayMapper::toDisplayData(const AirspaceEntity& entity)
{
    const QVariantMap styleMap = entity.style.toVariantMap();
    const QVariantMap propertiesMap = entity.properties.toVariantMap();

    AirspaceDraft tempDraft;
    tempDraft.geometry = entity.geometry;
    
    // Generate GeoJSON for rendering based on the exact specifications
    ShapeGenerator shapeGen;
    AirspaceGeometryConverter converter(&shapeGen);
    QJsonObject renderedGeoJson = converter.buildPreview(tempDraft).geoJson;

    return {
        { QStringLiteral("id"), entity.id },
        { QStringLiteral("name"), entity.name },
        { QStringLiteral("shapeType"), airspaceShapeTypeToInt(entity.geometry.shapeType) },
        { QStringLiteral("shapeTypeName"), airspaceShapeTypeDisplayName(entity.geometry.shapeType) },
        { QStringLiteral("geoJson"), AirspacePayloadConverter::toCompactJson(renderedGeoJson) },
        { QStringLiteral("geoJsonObject"), renderedGeoJson.toVariantMap() },
        { QStringLiteral("styleData"), styleMap },
        { QStringLiteral("propertiesData"), propertiesMap },
        { QStringLiteral("visible"), entity.visible },
        { QStringLiteral("createdAt"), entity.createdAt.toString(QStringLiteral("yyyy-MM-dd HH:mm")) },
        { QStringLiteral("updatedAt"), entity.updatedAt.toString(QStringLiteral("yyyy-MM-dd HH:mm")) }
    };
}