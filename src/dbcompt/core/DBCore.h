#pragma once
#include "../dbcompt_global.h"
#include "../schema/SchemaTypes.h"
#include "../driver/IDriver.h"
#include "../query/QueryResult.h"
#include <QString>
#include <QVariantMap>
#include <QVariantList>
#include <memory>

namespace dbcompt {

class DBCOMPT_EXPORT DBCore {
public:
    DBCore();
    ~DBCore();

    bool open(const QString& schemaFilePath, QString& outError);
    void close();
    bool isOpen()     const;
    QString lastError() const { return m_lastError; }

    // ── 便捷 CRUD ────────────────────────────────────────────────────────────
    QueryResult insert(const QString& table, const QVariantMap& values);

    QueryResult select(const QString& table,
                       const QVariantMap& conditions = {},
                       const QString& orderBy        = {},
                       int limit = -1);

    QueryResult update(const QString& table,
                       const QVariantMap& values,
                       const QVariantMap& conditions);

    QueryResult remove(const QString& table, const QVariantMap& conditions);

    // ── 原生 SQL ─────────────────────────────────────────────────────────────
    QueryResult rawQuery  (const QString& sql, const QVariantList& bindings = {});
    QueryResult rawExecute(const QString& sql, const QVariantList& bindings = {});

private:
    static std::unique_ptr<IDriver> createDriver(const QString& type);

    std::unique_ptr<IDriver> m_driver;
    SchemaDef                m_schema;
    QString                  m_lastError;
    bool                     m_open = false;
};

} // namespace dbcompt
