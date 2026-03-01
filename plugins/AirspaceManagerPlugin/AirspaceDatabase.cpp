#include "AirspaceDatabase.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QStandardPaths>
#include <QDir>
#include <QUuid>
#include <QDebug>

AirspaceDatabase::AirspaceDatabase(QObject* parent)
    : QObject(parent)
{
    // 数据库存放在 AppDataLocation/plugins/airspace-manager/airspaces.db
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    dataDir += "/plugins/airspace-manager";
    QDir().mkpath(dataDir);
    m_dbPath = dataDir + "/airspaces.db";
}

AirspaceDatabase::~AirspaceDatabase()
{
    close();
    {
        QString connName = "airspace_connection";
        m_db = QSqlDatabase();
        QSqlDatabase::removeDatabase(connName);
    }
}

bool AirspaceDatabase::open()
{
    if (m_open) return true;

    // 使用唯一连接名避免冲突
    m_db = QSqlDatabase::addDatabase("QSQLITE", "airspace_connection");
    m_db.setDatabaseName(m_dbPath);

    if (!m_db.open()) {
        qWarning() << "[AirspaceDB] Failed to open:" << m_db.lastError().text();
        return false;
    }

    if (!createTables()) {
        qWarning() << "[AirspaceDB] Failed to create tables";
        return false;
    }

    m_open = true;
    qDebug() << "[AirspaceDB] Opened at" << m_dbPath;
    return true;
}

void AirspaceDatabase::close()
{
    if (m_open) {
        m_db.close();
        m_open = false;
    }
}

bool AirspaceDatabase::isOpen() const
{
    return m_open;
}

bool AirspaceDatabase::createTables()
{
    QSqlQuery q(m_db);
    return q.exec(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS airspaces ("
        "  id           TEXT PRIMARY KEY,"
        "  name         TEXT NOT NULL,"
        "  shape_type   INTEGER NOT NULL,"
        "  geojson      TEXT NOT NULL,"
        "  style_json   TEXT DEFAULT '{}',"
        "  properties_json TEXT DEFAULT '{}',"
        "  visible      INTEGER DEFAULT 1,"
        "  created_at   TEXT NOT NULL,"
        "  updated_at   TEXT NOT NULL"
        ")"
    ));
}

QString AirspaceDatabase::generateUuid() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString AirspaceDatabase::addAirspace(const QString& name, int shapeType,
                                       const QString& geoJson,
                                       const QString& styleJson,
                                       const QString& propertiesJson)
{
    QString uuid = generateUuid();
    QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "INSERT INTO airspaces (id, name, shape_type, geojson, style_json, properties_json, visible, created_at, updated_at) "
        "VALUES (:id, :name, :st, :geo, :style, :props, 1, :ca, :ua)"
    ));
    q.bindValue(":id",    uuid);
    q.bindValue(":name",  name);
    q.bindValue(":st",    shapeType);
    q.bindValue(":geo",   geoJson);
    q.bindValue(":style", styleJson);
    q.bindValue(":props", propertiesJson);
    q.bindValue(":ca",    now);
    q.bindValue(":ua",    now);

    if (!q.exec()) {
        qWarning() << "[AirspaceDB] addAirspace failed:" << q.lastError().text();
        return {};
    }

    emit airspaceAdded(uuid);
    return uuid;
}

bool AirspaceDatabase::updateAirspace(const QString& id,
                                       const QString& name, int shapeType,
                                       const QString& geoJson,
                                       const QString& styleJson,
                                       const QString& propertiesJson)
{
    QString now = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "UPDATE airspaces SET name=:name, shape_type=:st, geojson=:geo, "
        "style_json=:style, properties_json=:props, updated_at=:ua WHERE id=:id"
    ));
    q.bindValue(":id",    id);
    q.bindValue(":name",  name);
    q.bindValue(":st",    shapeType);
    q.bindValue(":geo",   geoJson);
    q.bindValue(":style", styleJson);
    q.bindValue(":props", propertiesJson);
    q.bindValue(":ua",    now);

    if (!q.exec()) {
        qWarning() << "[AirspaceDB] updateAirspace failed:" << q.lastError().text();
        return false;
    }

    emit airspaceUpdated(id);
    return true;
}

bool AirspaceDatabase::removeAirspace(const QString& id)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("DELETE FROM airspaces WHERE id=:id"));
    q.bindValue(":id", id);

    if (!q.exec()) {
        qWarning() << "[AirspaceDB] removeAirspace failed:" << q.lastError().text();
        return false;
    }

    emit airspaceRemoved(id);
    return true;
}

AirspaceRecord AirspaceDatabase::getAirspace(const QString& id) const
{
    AirspaceRecord rec;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("SELECT * FROM airspaces WHERE id=:id"));
    q.bindValue(":id", id);

    if (q.exec() && q.next()) {
        rec.id             = q.value("id").toString();
        rec.name           = q.value("name").toString();
        rec.shapeType      = q.value("shape_type").toInt();
        rec.geoJson        = q.value("geojson").toString();
        rec.styleJson      = q.value("style_json").toString();
        rec.propertiesJson = q.value("properties_json").toString();
        rec.visible        = q.value("visible").toBool();
        rec.createdAt      = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
        rec.updatedAt      = QDateTime::fromString(q.value("updated_at").toString(), Qt::ISODate);
    }
    return rec;
}

QList<AirspaceRecord> AirspaceDatabase::getAllAirspaces() const
{
    QList<AirspaceRecord> list;
    QSqlQuery q(m_db);
    q.exec(QStringLiteral("SELECT * FROM airspaces ORDER BY created_at DESC"));

    while (q.next()) {
        AirspaceRecord rec;
        rec.id             = q.value("id").toString();
        rec.name           = q.value("name").toString();
        rec.shapeType      = q.value("shape_type").toInt();
        rec.geoJson        = q.value("geojson").toString();
        rec.styleJson      = q.value("style_json").toString();
        rec.propertiesJson = q.value("properties_json").toString();
        rec.visible        = q.value("visible").toBool();
        rec.createdAt      = QDateTime::fromString(q.value("created_at").toString(), Qt::ISODate);
        rec.updatedAt      = QDateTime::fromString(q.value("updated_at").toString(), Qt::ISODate);
        list.append(rec);
    }
    return list;
}

int AirspaceDatabase::count() const
{
    QSqlQuery q(m_db);
    q.exec(QStringLiteral("SELECT COUNT(*) FROM airspaces"));
    if (q.next()) return q.value(0).toInt();
    return 0;
}

bool AirspaceDatabase::setVisible(const QString& id, bool visible)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("UPDATE airspaces SET visible=:v WHERE id=:id"));
    q.bindValue(":v",  visible ? 1 : 0);
    q.bindValue(":id", id);

    if (!q.exec()) return false;

    emit airspaceUpdated(id);
    return true;
}

bool AirspaceDatabase::clearAll()
{
    QSqlQuery q(m_db);
    if (!q.exec(QStringLiteral("DELETE FROM airspaces"))) return false;
    emit databaseCleared();
    return true;
}
