#include "AirspaceMapLayerMapper.h"
#include "ShapeGenerator.h"
#include "AirspaceGeometryConverter.h"

QString AirspaceMapLayerMapper::layerId(const QString& airspaceId)
{
    return QStringLiteral("airspace-") + airspaceId;
}

QVariantMap AirspaceMapLayerMapper::baseStyle(const AirspaceEntity& entity)
{
    return entity.style.toVariantMap();
}

QVariantMap AirspaceMapLayerMapper::highlightedStyle(const QVariantMap& baseStyle)
{
    QVariantMap style = baseStyle;
    style[QStringLiteral("fill-color")] = style.value(QStringLiteral("fill-color"), QStringLiteral("#3388ff"));
    style[QStringLiteral("fill-opacity")] = 0.45;
    style[QStringLiteral("line-color")] = QStringLiteral("#ff8c1a");
    style[QStringLiteral("line-width")] = 4;
    style[QStringLiteral("line-dasharray")] = QVariantList{};
    return style;
}

AirspaceLayerUpdate AirspaceMapLayerMapper::toLayerUpdate(const AirspaceEntity& entity, bool selected)
{
    AirspaceDraft tempDraft;
    tempDraft.geometry = entity.geometry;
    ShapeGenerator shapeGen;
    AirspaceGeometryConverter converter(&shapeGen);

    AirspaceLayerUpdate update;
    update.layerId = layerId(entity.id);
    update.geoJson = converter.buildPreview(tempDraft).geoJson;
    update.style = selected ? highlightedStyle(baseStyle(entity)) : baseStyle(entity);
    return update;
}