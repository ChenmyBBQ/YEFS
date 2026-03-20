#ifndef AIRSPACEPAYLOADCONVERTER_H
#define AIRSPACEPAYLOADCONVERTER_H

#include <QJsonObject>
#include <QString>
#include <QVariant>

class AirspacePayloadConverter
{
public:
    static QJsonObject parseObject(const QString& json);
    static QVariantMap parseMap(const QString& json);
    static QString toCompactJson(const QJsonObject& object);
    static QString toCompactJson(const QVariantMap& map);
};

#endif // AIRSPACEPAYLOADCONVERTER_H