#include "AirspaceDatabase.h"

#include "AirspaceRecordMapper.h"

#include <DBCompt.h>
#include <QUuid>
#include <QDebug>

// ── 辅助函数 ──────────────────────────────────────────────────────────────────

AirspaceRecord AirspaceDatabase::rowToRecord(const QVariantMap& row)
{
    AirspaceRecord rec;
    rec.id             = row[QStringLiteral("id")].toString();
    rec.name           = row[QStringLiteral("name")].toString();
    rec.shapeType      = row[QStringLiteral("shape_type")].toInt();
    rec.geoJson        = row[QStringLiteral("geojson")].toString();
    rec.styleJson      = row[QStringLiteral("style_json")].toString();
    rec.propertiesJson = row[QStringLiteral("properties_json")].toString();
    rec.visible        = row[QStringLiteral("visible")].toBool();
    rec.createdAt      = QDateTime::fromString(row[QStringLiteral("created_at")].toString(), Qt::ISODate);
    rec.updatedAt      = QDateTime::fromString(row[QStringLiteral("updated_at")].toString(), Qt::ISODate);
    return rec;
}

QString AirspaceDatabase::generateUuid()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

// ── 构造 ──────────────────────────────────────────────────────────────────────

AirspaceDatabase::AirspaceDatabase(QObject* parent)
    : QObject(parent)
{}

// ── 生命周期 ──────────────────────────────────────────────────────────────────
// DBCompt 由主程序统一管理，此处 open/close 仅做状态查询委托

bool AirspaceDatabase::open()
{
    if (!DBCompt::instance()->isOpen()) {
        qWarning() << "[AirspaceDB] DBCompt 尚未打开，请确保主程序已初始化 DBCompt";
        return false;
    }
    qDebug() << "[AirspaceDB] ready (backed by DBCompt)";
    return true;
}

void AirspaceDatabase::close()
{
    // 由主程序统一关闭 DBCompt，此处无需操作
}

bool AirspaceDatabase::isOpen() const
{
    return DBCompt::instance()->isOpen();
}

QString AirspaceDatabase::databasePath() const
{
    // DBCompt 当前不暴露路径，返回固定说明即可
    return QStringLiteral("managed by DBCompt");
}

// ── CRUD ──────────────────────────────────────────────────────────────────────

QString AirspaceDatabase::addAirspace(const QString& name, int shapeType,
                                       const QString& geoJson,
                                       const QString& styleJson,
                                       const QString& propertiesJson)
{
    const QString uuid = generateUuid();
    const QString now  = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    const QVariantMap values{
        { QStringLiteral("id"),              uuid         },
        { QStringLiteral("name"),            name         },
        { QStringLiteral("shape_type"),      shapeType    },
        { QStringLiteral("geojson"),         geoJson      },
        { QStringLiteral("style_json"),      styleJson    },
        { QStringLiteral("properties_json"), propertiesJson },
        { QStringLiteral("visible"),         1            },
        { QStringLiteral("created_at"),      now          },
        { QStringLiteral("updated_at"),      now          }
    };

    const QString inserted = DBCompt::instance()->insert(TABLE, values);
    if (inserted.isEmpty()) {
        qWarning() << "[AirspaceDB] addAirspace failed:" << DBCompt::instance()->lastError();
        return {};
    }
    emit airspaceAdded(uuid);
    return uuid;
}

QString AirspaceDatabase::addAirspace(const AirspaceEntity& airspace)
{
    const AirspaceRecord record = AirspaceRecordMapper::toRecord(airspace);
    return addAirspace(record.name, record.shapeType, record.geoJson, record.styleJson, record.propertiesJson);
}

bool AirspaceDatabase::updateAirspace(const QString& id,
                                       const QString& name, int shapeType,
                                       const QString& geoJson,
                                       const QString& styleJson,
                                       const QString& propertiesJson)
{
    const QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    const QVariantMap values{
        { QStringLiteral("name"),            name          },
        { QStringLiteral("shape_type"),      shapeType     },
        { QStringLiteral("geojson"),         geoJson       },
        { QStringLiteral("style_json"),      styleJson     },
        { QStringLiteral("properties_json"), propertiesJson },
        { QStringLiteral("updated_at"),      now           }
    };
    const QVariantMap cond{ { QStringLiteral("id"), id } };

    if (!DBCompt::instance()->update(TABLE, values, cond)) {
        qWarning() << "[AirspaceDB] updateAirspace failed:" << DBCompt::instance()->lastError();
        return false;
    }
    emit airspaceUpdated(id);
    return true;
}

bool AirspaceDatabase::updateAirspace(const AirspaceEntity& airspace)
{
    const AirspaceRecord record = AirspaceRecordMapper::toRecord(airspace);
    return updateAirspace(record.id, record.name, record.shapeType,
        record.geoJson, record.styleJson, record.propertiesJson);
}

bool AirspaceDatabase::removeAirspace(const QString& id)
{
    const QVariantMap cond{ { QStringLiteral("id"), id } };
    if (!DBCompt::instance()->remove(TABLE, cond)) {
        qWarning() << "[AirspaceDB] removeAirspace failed:" << DBCompt::instance()->lastError();
        return false;
    }
    emit airspaceRemoved(id);
    return true;
}

AirspaceRecord AirspaceDatabase::getAirspace(const QString& id) const
{
    const QVariantMap cond{ { QStringLiteral("id"), id } };
    const QVariantList rows = DBCompt::instance()->select(TABLE, cond);
    if (rows.isEmpty())
        return {};
    return rowToRecord(rows.first().toMap());
}

AirspaceEntity AirspaceDatabase::getAirspaceEntity(const QString& id) const
{
    return AirspaceRecordMapper::toEntity(getAirspace(id));
}

QList<AirspaceRecord> AirspaceDatabase::getAllAirspaces() const
{
    const QVariantList rows = DBCompt::instance()->select(
        TABLE, {}, QStringLiteral("\"created_at\" DESC"));

    QList<AirspaceRecord> list;
    list.reserve(rows.size());
    for (const auto& rowVar : rows)
        list.append(rowToRecord(rowVar.toMap()));
    return list;
}

QList<AirspaceEntity> AirspaceDatabase::getAllAirspaceEntities() const
{
    const QList<AirspaceRecord> records = getAllAirspaces();
    QList<AirspaceEntity> entities;
    entities.reserve(records.size());
    for (const AirspaceRecord& record : records) {
        entities.append(AirspaceRecordMapper::toEntity(record));
    }
    return entities;
}

int AirspaceDatabase::count() const
{
    const QVariantList rows = DBCompt::instance()->rawQuery(
        QStringLiteral("SELECT COUNT(*) AS cnt FROM \"airspaces\""));
    if (!rows.isEmpty())
        return rows.first().toMap()[QStringLiteral("cnt")].toInt();
    return 0;
}

bool AirspaceDatabase::setVisible(const QString& id, bool visible)
{
    const QVariantMap values{ { QStringLiteral("visible"), visible ? 1 : 0 } };
    const QVariantMap cond  { { QStringLiteral("id"),      id              } };
    if (!DBCompt::instance()->update(TABLE, values, cond))
        return false;
    emit airspaceUpdated(id);
    return true;
}

bool AirspaceDatabase::clearAll()
{
    if (!DBCompt::instance()->rawExecute(
            QStringLiteral("DELETE FROM \"airspaces\"")))
        return false;
    emit databaseCleared();
    return true;
}
