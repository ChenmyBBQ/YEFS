#include "PostgresDriver.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

namespace dbcompt {

PostgresDriver::PostgresDriver()  = default;
PostgresDriver::~PostgresDriver() { close(); }

bool PostgresDriver::open(const DatabaseConfig& config, const QString& connectionName)
{
    m_connName = connectionName;
    m_db = QSqlDatabase::addDatabase(QStringLiteral("QPSQL"), m_connName);
    m_db.setHostName(config.host);
    m_db.setPort(config.port);
    m_db.setDatabaseName(config.dbname);
    m_db.setUserName(config.user);
    m_db.setPassword(config.password);

    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        return false;
    }
    m_open = true;
    return true;
}

void PostgresDriver::close()
{
    if (m_open) {
        m_db.close();
        m_open = false;
    }
    if (QSqlDatabase::contains(m_connName)) {
        m_db = QSqlDatabase();
        QSqlDatabase::removeDatabase(m_connName);
    }
}

bool PostgresDriver::isOpen() const { return m_open; }

bool PostgresDriver::execRaw(const QString& sql)
{
    QSqlQuery q(m_db);
    if (!q.exec(sql)) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

QueryResult PostgresDriver::query(const QString& sql, const QVariantList& bindings)
{
    QueryResult result;
    QSqlQuery q(m_db);
    q.prepare(sql);
    for (const auto& val : bindings)
        q.addBindValue(val);

    if (!q.exec()) {
        m_lastError  = q.lastError().text();
        result.error = m_lastError;
        return result;
    }

    const QSqlRecord rec = q.record();
    while (q.next()) {
        QVariantMap row;
        for (int i = 0; i < rec.count(); ++i)
            row[rec.fieldName(i)] = q.value(i);
        result.rows.append(row);
    }
    result.success = true;
    return result;
}

QueryResult PostgresDriver::execute(const QString& sql, const QVariantList& bindings)
{
    QueryResult result;
    QSqlQuery q(m_db);
    q.prepare(sql);
    for (const auto& val : bindings)
        q.addBindValue(val);

    if (!q.exec()) {
        m_lastError  = q.lastError().text();
        result.error = m_lastError;
        return result;
    }

    result.success      = true;
    result.lastInsertId = q.lastInsertId().toString();
    result.affectedRows = q.numRowsAffected();
    return result;
}

bool PostgresDriver::beginTransaction() { return m_db.transaction(); }
bool PostgresDriver::commit()           { return m_db.commit(); }
bool PostgresDriver::rollback()         { return m_db.rollback(); }

QStringList PostgresDriver::tableColumns(const QString& tableName)
{
    QStringList cols;
    auto res = query(
        QStringLiteral("SELECT column_name FROM information_schema.columns WHERE table_name = ?"),
        { tableName }
    );
    for (const auto& rowVar : res.rows)
        cols.append(rowVar.toMap()[QStringLiteral("column_name")].toString());
    return cols;
}

QStringList PostgresDriver::tables()
{
    return m_db.tables();
}

} // namespace dbcompt
