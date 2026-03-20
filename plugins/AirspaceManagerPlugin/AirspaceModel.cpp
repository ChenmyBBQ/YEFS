#include "AirspaceModel.h"

#include "AirspaceDisplayMapper.h"
#include "AirspacePayloadConverter.h"
#include "AirspaceRecordMapper.h"

#include <QDebug>
#include <QJsonDocument>

AirspaceModel::AirspaceModel(AirspaceDatabase* db, QObject* parent)
    : QAbstractListModel(parent)
    , m_db(db)
{
}

int AirspaceModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return m_records.size();
}

QVariant AirspaceModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_records.size())
        return {};

    const auto& r = m_records.at(index.row());
    switch (role) {
    case IdRole:             return r.id;
    case NameRole:           return r.name;
    case ShapeTypeRole:      return r.shapeType;
    case ShapeTypeNameRole:  return shapeTypeName(r.shapeType);
    case GeoJsonRole:        return r.geoJson;
    case GeoJsonObjectRole:  return AirspacePayloadConverter::parseMap(r.geoJson);
    case StyleJsonRole:      return r.styleJson;
    case StyleDataRole:      return AirspacePayloadConverter::parseMap(r.styleJson);
    case PropertiesJsonRole: return r.propertiesJson;
    case PropertiesDataRole: return AirspacePayloadConverter::parseMap(r.propertiesJson);
    case VisibleRole:        return r.visible;
    case CreatedAtRole:      return r.createdAt.toString("yyyy-MM-dd HH:mm");
    case UpdatedAtRole:      return r.updatedAt.toString("yyyy-MM-dd HH:mm");
    }
    return {};
}

QHash<int, QByteArray> AirspaceModel::roleNames() const
{
    return {
        { IdRole,             "airspaceId" },
        { NameRole,           "name" },
        { ShapeTypeRole,      "shapeType" },
        { ShapeTypeNameRole,  "shapeTypeName" },
        { GeoJsonRole,        "geoJson" },
        { GeoJsonObjectRole,  "geoJsonObject" },
        { StyleJsonRole,      "styleJson" },
        { StyleDataRole,      "styleData" },
        { PropertiesJsonRole, "propertiesJson" },
        { PropertiesDataRole, "propertiesData" },
        { VisibleRole,        "visible" },
        { CreatedAtRole,      "createdAt" },
        { UpdatedAtRole,      "updatedAt" }
    };
}

void AirspaceModel::loadFromDatabase()
{
    beginResetModel();
    m_records = m_db->getAllAirspaces();
    endResetModel();
    emit countChanged();
}

QString AirspaceModel::addAirspace(const QString& name, int shapeType,
                                    const QString& geoJson,
                                    const QString& styleJson,
                                    const QString& propertiesJson)
{
    QString uuid = m_db->addAirspace(name, shapeType, geoJson, styleJson, propertiesJson);
    if (uuid.isEmpty()) return {};

    // 获取刚插入的记录
    AirspaceRecord rec = m_db->getAirspace(uuid);

    beginInsertRows(QModelIndex(), 0, 0);
    m_records.prepend(rec);
    endInsertRows();

    emit countChanged();
    emit airspaceAdded(uuid);
    return uuid;
}

QString AirspaceModel::addAirspaceData(const QString& name, int shapeType,
                                        const QVariantMap& geoJsonObject,
                                        const QVariantMap& styleData,
                                        const QVariantMap& propertiesData)
{
    return addAirspace(
        name,
        shapeType,
    AirspacePayloadConverter::toCompactJson(geoJsonObject),
    AirspacePayloadConverter::toCompactJson(styleData),
    AirspacePayloadConverter::toCompactJson(propertiesData));
}

QString AirspaceModel::addAirspace(const AirspaceEntity& airspace)
{
    const AirspaceRecord record = AirspaceRecordMapper::toRecord(airspace);
    return addAirspace(record.name, record.shapeType, record.geoJson, record.styleJson, record.propertiesJson);
}

bool AirspaceModel::updateAirspace(const QString& id,
                                    const QString& name, int shapeType,
                                    const QString& geoJson,
                                    const QString& styleJson,
                                    const QString& propertiesJson)
{
    if (!m_db->updateAirspace(id, name, shapeType, geoJson, styleJson, propertiesJson))
        return false;

    int idx = findIndexById(id);
    if (idx >= 0) {
        m_records[idx] = m_db->getAirspace(id);
        emit dataChanged(index(idx), index(idx));
    }

    emit airspaceUpdated(id);
    return true;
}

bool AirspaceModel::updateAirspaceData(const QString& id,
                                        const QString& name, int shapeType,
                                        const QVariantMap& geoJsonObject,
                                        const QVariantMap& styleData,
                                        const QVariantMap& propertiesData)
{
    return updateAirspace(
        id,
        name,
        shapeType,
        AirspacePayloadConverter::toCompactJson(geoJsonObject),
        AirspacePayloadConverter::toCompactJson(styleData),
        AirspacePayloadConverter::toCompactJson(propertiesData));
}

bool AirspaceModel::updateAirspace(const AirspaceEntity& airspace)
{
    const AirspaceRecord record = AirspaceRecordMapper::toRecord(airspace);
    return updateAirspace(record.id, record.name, record.shapeType,
        record.geoJson, record.styleJson, record.propertiesJson);
}

bool AirspaceModel::removeAirspace(const QString& id)
{
    int idx = findIndexById(id);
    if (idx < 0) return false;

    if (!m_db->removeAirspace(id)) return false;

    beginRemoveRows(QModelIndex(), idx, idx);
    m_records.removeAt(idx);
    endRemoveRows();

    emit countChanged();
    emit airspaceRemoved(id);
    return true;
}

bool AirspaceModel::toggleVisibility(const QString& id)
{
    int idx = findIndexById(id);
    if (idx < 0) return false;

    bool newVis = !m_records[idx].visible;
    if (!m_db->setVisible(id, newVis)) return false;

    m_records[idx].visible = newVis;
    emit dataChanged(index(idx), index(idx), { VisibleRole });
    return true;
}

QVariantMap AirspaceModel::getAirspace(const QString& id) const
{
    const AirspaceEntity entity = getAirspaceEntity(id);
    if (entity.id.isEmpty()) {
        return {};
    }

    return AirspaceDisplayMapper::toDisplayData(entity);
}

AirspaceEntity AirspaceModel::getAirspaceEntity(const QString& id) const
{
    int idx = findIndexById(id);
    if (idx < 0) {
        return {};
    }

    return AirspaceRecordMapper::toEntity(m_records[idx]);
}

QString AirspaceModel::shapeTypeName(int type)
{
    return airspaceShapeTypeDisplayName(airspaceShapeTypeFromInt(type));
}

int AirspaceModel::findIndexById(const QString& id) const
{
    for (int i = 0; i < m_records.size(); ++i) {
        if (m_records[i].id == id) return i;
    }
    return -1;
}
