#pragma once
#include "../dbcompt_global.h"
#include "IDriver.h"
#include <QSqlDatabase>

namespace dbcompt {

// PostgreSQL/PostGIS 后端（预留接口，后续服务器/地图数据使用）
class DBCOMPT_EXPORT PostgresDriver : public IDriver {
public:
    PostgresDriver();
    ~PostgresDriver() override;

    bool open(const DatabaseConfig& config, const QString& connectionName) override;
    void close() override;
    bool isOpen() const override;

    bool        execRaw(const QString& sql) override;
    QueryResult query  (const QString& sql, const QVariantList& bindings = {}) override;
    QueryResult execute(const QString& sql, const QVariantList& bindings = {}) override;

    bool beginTransaction() override;
    bool commit()           override;
    bool rollback()         override;

    QString backendType()    const override { return QStringLiteral("postgres"); }
    QString connectionName() const override { return m_connName; }
    QString lastError()      const override { return m_lastError; }

    QStringList tableColumns(const QString& tableName) override;
    QStringList tables() override;

private:
    QSqlDatabase    m_db;
    QString         m_connName;
    mutable QString m_lastError;
    bool            m_open = false;
};

} // namespace dbcompt
