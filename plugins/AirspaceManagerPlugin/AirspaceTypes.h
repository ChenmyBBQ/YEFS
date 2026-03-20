#ifndef AIRSPACETYPES_H
#define AIRSPACETYPES_H

#include <QDateTime>
#include <QJsonObject>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QMap>
#include <QVector>

enum class AirspaceShapeType {
    Rectangle = 0,
    Square = 1,
    Circle = 2,
    Polygon = 3,
    Boundary = 4,
    Ring = 5,
    Arc = 6,
    Sector = 7,
    SectorRing = 8
};

struct AirspaceGeoPoint {
    double latitude = 0.0;
    double longitude = 0.0;

    QVariantList toVariantList() const;
    static AirspaceGeoPoint fromVariant(const QVariant& value);
};

struct AirspaceGeometrySpec {
    AirspaceShapeType shapeType = AirspaceShapeType::Rectangle;
    QVector<AirspaceGeoPoint> controlPoints;
    int previewSegments = 64;

    QVariantList toVariantList() const;
    QJsonObject toJsonObject() const;
    static AirspaceGeometrySpec fromJsonObject(const QJsonObject& json);
};

struct AirspaceDraft {
    AirspaceGeometrySpec geometry;
    bool hasHoverPoint = false;
    AirspaceGeoPoint hoverPoint;
    QVariantMap shapeInfo;

    void clear();
    QVariantList collectedPoints() const;
    QVariantList previewPoints() const;
};

struct AirspaceStyle {
    QString fillColor = QStringLiteral("#3388ff");
    double fillOpacity = 0.3;
    QString lineColor = QStringLiteral("#3388ff");
    double lineWidth = 2.0;

    QVariantMap toVariantMap() const;
    static AirspaceStyle fromVariantMap(const QVariantMap& map);
};

struct AirspaceProperties {
    int airspaceType = 0;
    double minAltitude = 0.0;
    double maxAltitude = 1000.0;
    QString effectiveTime;
    QString expirationTime;
    QString remarks;

    QVariantMap toVariantMap() const;
    static AirspaceProperties fromVariantMap(const QVariantMap& map);
};

struct AirspaceEntity {
    QString id;
    QString name;
    AirspaceGeometrySpec geometry;
    AirspaceStyle style;
    AirspaceProperties properties;
    bool visible = true;
    QDateTime createdAt;
    QDateTime updatedAt;
};

AirspaceShapeType airspaceShapeTypeFromInt(int value);
int airspaceShapeTypeToInt(AirspaceShapeType value);
QString airspaceShapeTypeDisplayName(AirspaceShapeType value);

#endif // AIRSPACETYPES_H