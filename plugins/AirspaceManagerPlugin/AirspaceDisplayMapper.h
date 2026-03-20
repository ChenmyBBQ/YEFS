#ifndef AIRSPACEDISPLAYMAPPER_H
#define AIRSPACEDISPLAYMAPPER_H

#include "AirspaceTypes.h"

#include <QVariant>

class AirspaceDisplayMapper
{
public:
    static QVariantMap toDisplayData(const AirspaceEntity& entity);
};

#endif // AIRSPACEDISPLAYMAPPER_H