#include "AirspaceRecordMapper.h"

#include "AirspacePayloadConverter.h"

AirspaceEntity AirspaceRecordMapper::toEntity(const AirspaceRecord& record)
{
    AirspaceEntity entity;
    entity.id = record.id;
    entity.name = record.name;
    entity.geometry = AirspaceGeometrySpec::fromJsonObject(AirspacePayloadConverter::parseObject(record.geoJson));
    entity.style = AirspaceStyle::fromVariantMap(AirspacePayloadConverter::parseObject(record.styleJson).toVariantMap());
    entity.properties = AirspaceProperties::fromVariantMap(AirspacePayloadConverter::parseObject(record.propertiesJson).toVariantMap());
    entity.visible = record.visible;
    entity.createdAt = record.createdAt;
    entity.updatedAt = record.updatedAt;
    return entity;
}

AirspaceRecord AirspaceRecordMapper::toRecord(const AirspaceEntity& entity)
{
    AirspaceRecord record;
    record.id = entity.id;
    record.name = entity.name;
    record.shapeType = airspaceShapeTypeToInt(entity.geometry.shapeType);
    record.geoJson = AirspacePayloadConverter::toCompactJson(entity.geometry.toJsonObject());
    record.styleJson = AirspacePayloadConverter::toCompactJson(QJsonObject::fromVariantMap(entity.style.toVariantMap()));
    record.propertiesJson = AirspacePayloadConverter::toCompactJson(QJsonObject::fromVariantMap(entity.properties.toVariantMap()));
    record.visible = entity.visible;
    record.createdAt = entity.createdAt;
    record.updatedAt = entity.updatedAt;
    return record;
}