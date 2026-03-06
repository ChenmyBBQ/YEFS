#include "SQLiteDriver.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDir>
#include <QFileInfo>

namespace dbcompt {

SQLiteDriver::SQLiteDriver()  = default;
SQLiteDriver::~SQLiteDriver() { close(); }

bool SQLiteDriver::open(const DatabaseConfig& config, const QString& connectionName)
{
    m_connName = connectionName;

    // 确保目录存在
    QDir().mkpath(QFileInfo(config.path).absolutePath());

    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_connName);
    m_db.setDatabaseName(config.path);

    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        return false;
    }

    // 开启 WAL 模式提升并发性能，开启外键约束
    QSqlQuery q(m_db);
    q.exec(QStringLiteral("PRAGMA journal_mode=WAL"));
    q.exec(QStringLiteral("PRAGMA foreign_keys=ON"));

    m_open = true;
    return true;
}

void SQLiteDriver::close()
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

bool SQLiteDriver::isOpen() const { return m_open; }

bool SQLiteDriver::execRaw(const QString& sql)
{
    QSqlQuery q(m_db);
    if (!q.exec(sql)) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

QueryResult SQLiteDriver::query(const QString& sql, const QVariantList& bindings)
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

QueryResult SQLiteDriver::execute(const QString& sql, const QVariantList& bindings)
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

bool SQLiteDriver::beginTransaction() { return m_db.transaction(); }
bool SQLiteDriver::commit()           { return m_db.commit(); }
bool SQLiteDriver::rollback()         { return m_db.rollback(); }

QStringList SQLiteDriver::tableColumns(const QString& tableName)
{
    QStringList cols;
    QSqlQuery q(m_db);
    q.exec(QStringLiteral("PRAGMA table_info(") + tableName + QStringLiteral(")"));
    while (q.next())
        cols.append(q.value(QStringLiteral("name")).toString());
    return cols;
}

QStringList SQLiteDriver::tables()
{
    return m_db.tables();
}

} // namespace dbcompt
