#ifndef AIRSPACEDRAFTVALIDATOR_H
#define AIRSPACEDRAFTVALIDATOR_H

#include "AirspaceTypes.h"

class AirspaceDraftValidator
{
public:
    static int minimumPoints(AirspaceShapeType shapeType);
    static bool canPreview(const AirspaceDraft& draft);
    static bool canFinish(const AirspaceDraft& draft);
    static QString finishError(const AirspaceDraft& draft);
};

#endif // AIRSPACEDRAFTVALIDATOR_H