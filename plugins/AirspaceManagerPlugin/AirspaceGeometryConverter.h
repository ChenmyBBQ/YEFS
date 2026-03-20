#ifndef AIRSPACEGEOMETRYCONVERTER_H
#define AIRSPACEGEOMETRYCONVERTER_H

#include "AirspaceTypes.h"

#include <QJsonObject>
#include <QVariantList>

class ShapeGenerator;

struct AirspacePreviewResult {
    QJsonObject geoJson;
    QVariantMap shapeInfo;
    QVariantList annotationPoints;
};

class AirspaceGeometryConverter
{
public:
    explicit AirspaceGeometryConverter(const ShapeGenerator* shapeGenerator = nullptr);

    void setShapeGenerator(const ShapeGenerator* shapeGenerator);
    AirspacePreviewResult buildPreview(const AirspaceDraft& draft) const;

private:
    static QVariantList extractAnnotationPoints(const QJsonObject& geoJson);
    static double haversineDistance(double lat1, double lng1, double lat2, double lng2);
    static double bearingTo(double lat1, double lng1, double lat2, double lng2);

    const ShapeGenerator* m_shapeGenerator = nullptr;
};

#endif // AIRSPACEGEOMETRYCONVERTER_H