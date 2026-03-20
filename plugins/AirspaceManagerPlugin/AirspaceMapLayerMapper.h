#ifndef AIRSPACEMAPLAYERMAPPER_H
#define AIRSPACEMAPLAYERMAPPER_H

#include "AirspaceTypes.h"

#include <QJsonObject>
#include <QString>
#include <QVariant>

struct AirspaceLayerUpdate {
    QString layerId;
    QJsonObject geoJson;
    QVariantMap style;
};

class AirspaceMapLayerMapper
{
public:
    static QString layerId(const QString& airspaceId);
    static QVariantMap baseStyle(const AirspaceEntity& entity);
    static QVariantMap highlightedStyle(const QVariantMap& baseStyle);
    static AirspaceLayerUpdate toLayerUpdate(const AirspaceEntity& entity, bool selected);
};

#endif // AIRSPACEMAPLAYERMAPPER_H