#include "AirspacePayloadConverter.h"

#include <QJsonDocument>

QJsonObject AirspacePayloadConverter::parseObject(const QString& json)
{
    return QJsonDocument::fromJson(json.toUtf8()).object();
}

QVariantMap AirspacePayloadConverter::parseMap(const QString& json)
{
    return parseObject(json).toVariantMap();
}

QString AirspacePayloadConverter::toCompactJson(const QJsonObject& object)
{
    return QString::fromUtf8(QJsonDocument(object).toJson(QJsonDocument::Compact));
}

QString AirspacePayloadConverter::toCompactJson(const QVariantMap& map)
{
    return toCompactJson(QJsonObject::fromVariantMap(map));
}