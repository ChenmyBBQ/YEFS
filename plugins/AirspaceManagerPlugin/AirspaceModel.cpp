#include "AirspaceModel.h"
#include <QDebug>

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
    case StyleJsonRole:      return r.styleJson;
    case PropertiesJsonRole: return r.propertiesJson;
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
        { StyleJsonRole,      "styleJson" },
        { PropertiesJsonRole, "propertiesJson" },
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
    int idx = findIndexById(id);
    if (idx < 0) return {};

    const auto& r = m_records[idx];
    return {
        { "id",             r.id },
        { "name",           r.name },
        { "shapeType",      r.shapeType },
        { "shapeTypeName",  shapeTypeName(r.shapeType) },
        { "geoJson",        r.geoJson },
        { "styleJson",      r.styleJson },
        { "propertiesJson", r.propertiesJson },
        { "visible",        r.visible },
        { "createdAt",      r.createdAt.toString("yyyy-MM-dd HH:mm") },
        { "updatedAt",      r.updatedAt.toString("yyyy-MM-dd HH:mm") }
    };
}

QString AirspaceModel::shapeTypeName(int type)
{
    switch (type) {
    case 0: return QStringLiteral("矩形");
    case 1: return QStringLiteral("正方形");
    case 2: return QStringLiteral("圆形");
    case 3: return QStringLiteral("多边形");
    case 4: return QStringLiteral("边界线");
    case 5: return QStringLiteral("圆环");
    case 6: return QStringLiteral("圆弧");
    case 7: return QStringLiteral("扇形");
    case 8: return QStringLiteral("扇环形");
    default: return QStringLiteral("未知");
    }
}

int AirspaceModel::findIndexById(const QString& id) const
{
    for (int i = 0; i < m_records.size(); ++i) {
        if (m_records[i].id == id) return i;
    }
    return -1;
}
